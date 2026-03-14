#include "runtime_exports.h"

#include "image_impl.h"

using namespace std;
using namespace std::chrono;
using namespace r_code;
using namespace r_comp;

void decompile_image(Decompiler& decompiler, r_comp::Image* image, Timestamp time_reference, bool ignore_named_objects) {

#ifdef DECOMPILE_ONE_BY_ONE
  uint32 object_count = decompiler.decompile_references(image);
  std::cout << object_count << " objects in the image\n";
  while (1) {

    std::cout << "> which object (-1 to exit)?\n";
    int32 index; std::cin >> index;
    if (index == -1)
      break;
    if (index >= object_count) {

      std::cout << "> there is only " << object_count << " objects\n";
      continue;
    }
    std::ostringstream decompiled_code;
    decompiler.decompile_object(index, &decompiled_code, time_reference);
    std::cout << "\n\n> DECOMPILATION. TimeReference "
      << Utils::ToString_s_ms_us(time_reference, Timestamp(seconds(0)))
      << "\n\n" << decompiled_code.str() << std::endl;
  }
#else
  std::ostringstream decompiled_code;
  uint32 object_count = decompiler.decompile(image, &decompiled_code, time_reference, ignore_named_objects);
  std::cout << "\n\n> DECOMPILATION. TimeReference "
    << Utils::ToString_s_ms_us(time_reference, Timestamp(seconds(0)))
    << "\n\n" << decompiled_code.str() << std::endl;
  std::cout << "> image taken at: " << Time::ToString_year(image->timestamp_) << std::endl;
  std::cout << "> " << object_count << " objects\n";
#endif
}

void write_image_to_file(r_comp::Image* image, const std::string& image_path, Decompiler* decompiler, Timestamp time_reference) {

  ofstream output(image_path.c_str(), ios::binary | ios::out);
  r_code::Image<r_code::ImageImpl>* serialized_image = image->serialize<r_code::Image<r_code::ImageImpl> >();
  r_code::Image<r_code::ImageImpl>::Write(serialized_image, output);
  output.close();
  delete serialized_image;

  if (!decompiler)
    return;

  ifstream input(image_path.c_str(), ios::binary | ios::in);
  if (!input.good())
    return;

  r_code::Image<r_code::ImageImpl>* read_image =
    (r_code::Image<r_code::ImageImpl>*)r_code::Image<r_code::ImageImpl>::Read(input);
  input.close();

  r_comp::Image* temp_image = new r_comp::Image();
  temp_image->load(read_image);

  decompile_image(*decompiler, temp_image, time_reference, false);
  delete temp_image;
  delete read_image;
}
