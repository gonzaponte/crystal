#include "actions.hh"
#include "config.hh"
#include "geometry.hh"
#include "materials.hh"

#include <n4-inspect.hh>
#include <n4-material.hh>
#include <n4-sensitive.hh>
#include <n4-sequences.hh>
#include <n4-shape.hh>

#include <G4OpticalSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4TrackStatus.hh>

G4PVPlacement* crystal_geometry(run_stats& stats) {
  auto scintillator = scintillator_material(my.scint_params().scint);
  auto air     = n4::material("G4_AIR");
  auto vacuum  = n4::material("G4_Galactic");
  auto silicon = silicon_with_properties();
  auto teflon  =  teflon_with_properties();

  auto [sx, sy, sz] = n4::unpack(my.scint_size());

  auto world  = n4::box("world").xyz(sx*1.5, sy*1.5, sz*2.5).place(air).now();
  auto reflector = n4::box("reflector")
    .x(sx + 2*my.reflector_thickness)
    .y(sy + 2*my.reflector_thickness)
    .z(sz                           )
    .place(teflon).at_z(-sz/2)
    .in(world).now();

  n4::box("absorber")
    .x(sx + 2*my.reflector_thickness)
    .y(sy + 2*my.reflector_thickness)
    .z(       my.reflector_thickness)
    .place(vacuum).at_z(-sz -my.reflector_thickness / 2)
    .in(world).now();

  auto crystal = n4::box("crystal")
    .xyz(sx, sy, sz)
    .place(scintillator)
    .in(reflector).now();

  auto process_hits = [&stats] (G4Step* step) {
    static auto optical_photon = n4::find_particle("opticalphoton");
    auto track = step -> GetTrack();
    if (track -> GetDefinition() == optical_photon) {
      stats.n_detected_evt++;
      track -> SetTrackStatus(fStopAndKill);
      size_t n = step -> GetPreStepPoint() -> GetPhysicalVolume() -> GetCopyNo();
      ++stats.n_detected_at_sipm[n];
    }
    return true;
  };

  auto sipm = n4::box("sipm")
    .xy(my.scint_params().sipm_size).z(my.sipm_thickness)
    .sensitive("sipm", process_hits)
    .place(silicon).in(world);

  auto n=0;
  for (const auto& pos: my.sipm_positions()) {
      sipm.clone().at(pos).copy_no(n++).now();
  }

  // TODO add abstraction for placing optical surface between volumes
  auto
  teflon_surface = new G4OpticalSurface("teflon_surface");
  teflon_surface -> SetType(dielectric_dielectric);
  teflon_surface -> SetModel(unified);
  teflon_surface -> SetFinish(groundfrontpainted);     // Lambertian
  //teflon_surface -> SetFinish(polishedfrontpainted); // Specular
  teflon_surface -> SetSigmaAlpha(0.0);

  teflon_surface -> SetMaterialPropertiesTable(teflon_properties());
  new G4LogicalBorderSurface("teflon_surface", crystal, reflector, teflon_surface);

  return world;
}
