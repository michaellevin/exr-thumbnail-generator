#include <iostream>
#include <OpenImageIO/imageio.h>
#include <string>
#include <vector>
#include "argparse.hpp"

struct ArgumentParser : public argparse::Args
{
    std::string& src_path = arg("source path");
    std::string& tgt_path = arg("target path");
    int& lowres_width = kwarg("w,lowres-width", "Lowres width sise").set_default(512);
    float& gamma = kwarg("g,gamma", "Gamma correction value").set_default(1.8f);
    bool& verbose = flag("v,verbose", "Verbose mode");
};

bool reverse_compare(double a, double b)
{
    return a > b;
}

void normalize_image(std::vector<float>& pixels)
{

    std::vector<float> buffer;
    buffer.assign(pixels.begin(), pixels.end());
    std::sort(buffer.begin(), buffer.end(), reverse_compare);
    float scale_factor = buffer[floor(0.01 * buffer.size())];

    for (int i = 0; i < pixels.size(); ++i)
    {

        pixels[i] = pixels[i] == 0 ? 0 : pixels[i] / scale_factor;
    }
}

void apply_gamma(std::vector<float>& pixels, float gamma)
{
    for (int i = 0; i < pixels.size(); ++i)
    {

        pixels[i] = pow(pixels[i], (1.0f / gamma));
    }
}

bool generate_thumbnail(
    std::string& source_path,
    std::string& target_path,
    int lowres_width = 512,
    float gamma = 2.2,
    bool verbose = false)
{

    auto in_file = OIIO::ImageInput::open(source_path);

    if (!in_file)
    {
        std::cerr << "Source file is invalid or does not exist!" << std::endl;
        return false;
    }

    const OIIO::ImageSpec& file_spec = in_file->spec();

    // calculate lowres height
    int lowres_height = float(file_spec.height) / float(file_spec.width) * lowres_width;

    if (file_spec.tile_width == 0)
    {
        std::cout << "Image is scanlined. Ok!" << std::endl;

        std::vector<float> thumb_pixels(lowres_width * lowres_height * file_spec.nchannels);

        std::vector<float> scanline(file_spec.width * file_spec.nchannels);

        // reshaping
        int lowres_pixel_index = 0;
        for (int y = 0; y < lowres_height; ++y)
        {
            // read highres scanline
            int highres_line_index = round(float(y) / float(lowres_height) * float(file_spec.height));

            in_file->read_scanline(highres_line_index, 0, OIIO::TypeDesc::FLOAT, &scanline[0]);

            for (int x = 0; x < lowres_width; ++x)
            {

                int highres_line_pixel_index = round(float(x) / float(lowres_width) * float(file_spec.width)) * file_spec.nchannels;

                for (int chnl = 0; chnl < file_spec.nchannels; ++chnl)
                {
                    thumb_pixels[lowres_pixel_index + chnl] = scanline[highres_line_pixel_index + chnl];
                    //  * color_amount, (1.0 / gamma)
                }

                lowres_pixel_index += file_spec.nchannels;
            }
        }


        normalize_image(thumb_pixels);
        // std::cout << "Gamma set to " << gamma << std::endl;
        apply_gamma(thumb_pixels, gamma);

        // Converting to jpg
        auto out_file = OIIO::ImageOutput::create(target_path);
        if (!out_file)
        {
            std::cerr << "Target file is invalid or does not exist!" << std::endl;
            return false;
        }

        OIIO::ImageSpec thumb_spec(lowres_width, lowres_height, file_spec.nchannels, OIIO::TypeDesc::FLOAT);
        out_file->open(target_path, thumb_spec);
        out_file->write_image(OIIO::TypeDesc::FLOAT, thumb_pixels.data());
        out_file->close();
    }

    else
    {
        std::cout << "Image is tiled! Cannot do anymore" << std::endl;
    }

    in_file->close();

    return true;
}

int main(int argc, char* argv[])
{
    ArgumentParser args = argparse::parse<ArgumentParser>(argc, argv);

    if (!generate_thumbnail(
        args.src_path,
        args.tgt_path,
        args.lowres_width,
        args.gamma,
        args.verbose))
    {
        return 1;
    }

    return 0;
}
