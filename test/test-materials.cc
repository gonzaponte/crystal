#include <config.hh>
#include <geometry.hh>
#include <materials.hh>
#include <physics-list.hh>

#include <n4-all.hh>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Step.hh>
#include <G4ThreeVector.hh>
#include <G4TrackStatus.hh>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;



auto blue_light_towards_teflon() {
  auto particle_type = n4::find_particle("opticalphoton");
  auto energy        = 2.5 * eV;
  auto [x, y, z] = unpack(my.scint_size);

  // Avoid the SiPM face which is not covered by teflon
  auto min_theta = std::atan(std::hypot(x,y) / z);
  auto random_direction = n4::random::direction{}.min_theta(min_theta);

  return [energy, z, particle_type, random_direction] (G4Event* event) {
    auto r        = random_direction.get() * energy;
    auto particle = new G4PrimaryParticle{particle_type, r.x(), r.y(), r.z()};
    particle -> SetPolarization(random_direction.get());
    auto vertex   = new G4PrimaryVertex{{0,0,-z/2}, 0};
    vertex -> SetPrimary(particle);
    event  -> AddPrimaryVertex(vertex);
  };
}

// Check whether step exists and if so, whether its value is as specified
bool step_matches(std::vector<G4LogicalVolume*>& step_volumes, size_t pos, G4LogicalVolume* value) {
    return step_volumes.size() > pos && step_volumes[pos] == value;
  };

// Shoot photons from within crystal in random directions towards teflon
// reflector (avoiding the SiPM face which is not covered by teflon). Count
// events in which the photon reaches the teflon in the first step. Cound how
// many of those result in the photon returning into the crystal in the next
// step. Check that the ratio of these counts is equal to the expected
// reflectivity.
TEST_CASE("csi teflon reflectivity fraction", "[csi][teflon][reflectivity]") {
  unsigned count_incoming = 0, count_reflected = 0;

  std::vector<G4LogicalVolume*> step_volumes; // Space for storing the first and second volumes to be stepped through

  // Stepping action: record the volumes visited in the steps
  auto record_step_volumes = [&step_volumes] (const G4Step* step) {
    auto track = step -> GetTrack();
    auto next_volume = track -> GetNextVolume() -> GetLogicalVolume();
    step_volumes.push_back(next_volume);
    if (step_volumes.size() >= 2) { track -> SetTrackStatus(fStopAndKill); } // The first 2 steps tell us all we need to know
  };

  // Begin of event action: forget volumes visited in the previous event
  auto reset_step_list = [&step_volumes] (const G4Event*) { step_volumes.clear(); };

  // End of event action: count events where
  // 1. reflector was reached in first step
  // 2. second step resulted in reflection
  auto classify_events = [&] (const G4Event*) {
    auto reflector = n4::find_logical("reflector");
    auto crystal   = n4::find_logical("crystal");
    if (step_matches(step_volumes, 0, reflector)) { count_incoming ++; } else { return; }
    if (step_matches(step_volumes, 1, crystal  )) { count_reflected++; }
  };

  // Gather together all the above actions
  auto test_action = [&] {
    return (new n4::actions{blue_light_towards_teflon()})
      -> set((new n4::event_action)
             -> begin(reset_step_list)
             ->   end(classify_events))
      -> set(new n4::stepping_action{record_step_volumes})
      ;
  };

  n4::run_manager::create()
    .fake_ui()
    .physics(physics_list)
    .geometry(crystal_geometry)
    .actions(test_action)
    .run(100000);

  auto teflon_reflectivity_percentage   = 98;
  auto measured_reflectivity_percentage = 100.0 * count_reflected / count_incoming;

  std::cout << "\n\n-------------------- PHOTONS HITTING BOUNDARY    : " << count_incoming
            << "   reflected: " << count_reflected << "   ratio: " <<  measured_reflectivity_percentage << " %" << std::endl;

  CHECK_THAT(measured_reflectivity_percentage, WithinRel(teflon_reflectivity_percentage, 1e-3));
}


TEST_CASE("csi teflon reflectivity lambertian", "[csi][teflon][reflectivity]") {
  std::vector<G4LogicalVolume*> step_volumes;
  std::vector<G4double> step_thetas;
  std::vector<G4double> theta_in, theta_out;

  auto record_data = [&step_volumes, &step_thetas] (const G4Step* step) {
    auto track = step -> GetTrack();
    if ( track -> GetCurrentStepNumber() < 3 ) {
      auto next_volume = track -> GetNextVolume() -> GetLogicalVolume();
      step_volumes.push_back(next_volume);
      auto pos = step -> GetPostStepPoint() -> GetPosition();
      auto p = step -> GetPreStepPoint()  -> GetMomentumDirection();
      auto n = WithinRel(-my.scint_size.z()  , 1e-6).match(pos.z()) ? G4ThreeVector{ 0,  0,  1}
             : WithinRel(-my.scint_size.x()/2, 1e-6).match(pos.x()) ? G4ThreeVector{ 1,  0,  0}
             : WithinRel( my.scint_size.x()/2, 1e-6).match(pos.x()) ? G4ThreeVector{-1,  0,  0}
             : WithinRel(-my.scint_size.y()/2, 1e-6).match(pos.y()) ? G4ThreeVector{ 0,  1,  0}
             : WithinRel( my.scint_size.y()/2, 1e-6).match(pos.y()) ? G4ThreeVector{ 0, -1,  0}
             :                                                        G4ThreeVector{ 0,  0, -1}; // Meaningless
      auto theta = std::acos(p.dot(n));
      step_thetas.push_back(theta);
    }
    else {
      step -> GetTrack() -> SetTrackStatus(fStopAndKill);
    }
  };

  auto reset_data = [&step_volumes, &step_thetas] (const G4Event*) {
    step_volumes.clear();
    step_thetas .clear();
  };

  auto check_validity = [&step_volumes, &step_thetas, &theta_in, &theta_out] (const G4Event* event) {
    auto crystal   = n4::find_logical("crystal");
    auto reflector = n4::find_logical("reflector");
    if (step_matches(step_volumes, 0, reflector) &&
        step_matches(step_volumes, 1, crystal)) {
      theta_in .push_back(step_thetas[0]);
      theta_out.push_back(step_thetas[1]);
    }
  };

  auto test_action = [&] {
    return (  new n4::actions{blue_light_towards_teflon()})
      -> set((new n4::   event_action{}) -> begin(reset_data) -> end(check_validity))
      -> set( new n4::stepping_action{record_data})
      ;
  };

  n4::run_manager::create()
    .fake_ui()
    .physics(physics_list)
    .geometry(crystal_geometry)
    .actions(test_action)
    .run(100000);

  auto n = theta_in.size();

  auto mean_theta_in  = n4::stats::mean(theta_in ).value();
  auto mean_theta_out = n4::stats::mean(theta_out).value();

  std::vector<double> dtheta_in, dtheta_out;
  dtheta_in  .reserve(n);
  dtheta_out .reserve(n);

  std::for_each(std::begin(theta_in ), std::end(theta_in ), [&] (auto th) { dtheta_in .push_back(th - mean_theta_in ); });
  std::for_each(std::begin(theta_out), std::end(theta_out), [&] (auto th) { dtheta_out.push_back(th - mean_theta_out); });

  auto sigma_in  = n4::stats::std_dev_sample(dtheta_in ).value();
  auto sigma_out = n4::stats::std_dev_sample(dtheta_out).value();

  auto correlation = 0.;
  for (auto k=0; k<n; k++) { correlation += dtheta_in[k] * dtheta_out[k]; }
  correlation /= n * sigma_in * sigma_out;

  CHECK_THAT(correlation, WithinAbs(0, 1e-2));
}
