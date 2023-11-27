#include <materials.hh>
#include <physics-list.hh>

#include <n4-sequences.hh>
#include <n4-stream.hh>
#include <n4-will-become-external-lib.hh>

#include <G4Material.hh>
#include <G4SystemOfUnits.hh>
#include <G4UnitsTable.hh>

#include <iomanip>

std::vector<double> abslengths(const std::string name, G4Material* material, std::vector<double> distances) {
  auto abs_lengths = measure_abslength(test_config{ .physics         = physics_list()
                                                  , .material        = material
                                                  , .particle_name   = "gamma"
                                                  , .particle_energy = 511 * keV
                                                  , .distances       = distances});

  return abs_lengths;
}

void fractions(const std::string& material_name, G4Material* material) {
  std::cout << "Calculating interaction fractions for : '" << material_name << "'\n";
  auto fractions = [&] {
    n4::silence hush{std::cout};
    return calculate_interaction_process_fractions(material, physics_list());
  }();
  using std::setw; using std::setprecision; using std::fixed;
  std::cout
      << "\nphotoelectric: " << fixed << setw(4) << setprecision(1) << fractions.photoelectric * 100 << '%'   << std::endl
      <<   "compton      : " << fixed << setw(4) << setprecision(1) << fractions.compton       * 100 << '%'   << std::endl
      <<   "rayleigh     : " << fixed << setw(4) << setprecision(1) << fractions.rayleigh      * 100 << "%\n" << std::endl;
}

void abslength(const std::string& material_name, G4Material* material) {

  auto distances = n4::scale_by(mm, {5, 10, 15, 20, 25, 30, 35, 40, 45, 50});
  std::cout << "Calculating attenuation lengths for : '" << material_name << "'\n"
            << distances.size() << " statistical samples ...\n\n";
  std::string name;
  auto results = [&] {
    n4::silence hush{std::cout};
    return abslengths(name, material, distances);
  }();
  for (const auto& d: results) { std::cout << G4BestUnit(d, "Length") << ' '; }
  std::cout << std::endl << std::endl << std::endl;

}


int main(int argc, char** argv) {

  auto material_choice = std::string{argv[2]};
  std::string name;
  G4Material* material;

  if      (material_choice == "csi" ) { name = "CsI" ; material =  csi_with_properties(); }
  else if (material_choice == "bgo" ) { name = "BGO" ; material =  bgo_with_properties(); }
  else if (material_choice == "lyso") { name = "LYSO"; material = lyso_with_properties(); }
  else                                { std::cerr << "Unknown material " << material_choice << std::endl; exit(1); }

  auto task_choice = std::string{argv[1]};
  if      (task_choice == "fractions") { fractions(name, material); }
  else if (task_choice == "abslength") { abslength(name, material); }

}
