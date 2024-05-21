#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>

// Function to create a directory if it doesn't exist
bool create_directory(const std::string& path) {
    #ifdef _WIN32
    int result = _mkdir(path.c_str());
    #else
    mode_t mode = 0755;
    int result = mkdir(path.c_str(), mode);
    #endif
    return (result == 0);
}

// Function to check if a directory exists
bool directory_exists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

struct FitParameters {
    std::string path;
    std::string filename;
    std::string suffix;
    std::string y_min;
    std::string y_max;
    std::string x_min;
    std::string x_max;
};

std::vector<FitParameters> parse_csv(const std::string& filepath) {
    std::vector<FitParameters> parameter_sets;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open CSV file." << std::endl;
        return parameter_sets;
    }

    std::string line;
    std::getline(file, line); // Skip the header row

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        FitParameters params;
        std::getline(ss, params.path, ',');
        std::getline(ss, params.filename, ',');
        std::getline(ss, params.suffix, ',');
        std::getline(ss, params.y_min, ',');
        std::getline(ss, params.y_max, ',');
        std::getline(ss, params.x_min, ',');
        std::getline(ss, params.x_max, ',');
        parameter_sets.push_back(params);
    }

    return parameter_sets;
}

void perform_curve_fit(const FitParameters& params, std::ofstream& output_csv) {
    // Create directory to store PNG files or output to it if it exists
    std::string png_directory = "plots_" + params.filename;
    if (!directory_exists(png_directory)) {
        if (!create_directory(png_directory)) {
            std::cerr << "Error creating directory: " << png_directory << std::endl;
            return;
        }
    }

    // Update filename to include directory path
    std::string png_filename = png_directory + "/plot_" + params.filename + ".png";

    // Create a temporary file for Gnuplot output
    std::string tmp_filename = "gnuplot_output.tmp";
    FILE* gnuplotPipe = popen("gnuplot", "w");
    if (!gnuplotPipe) {
        std::cerr << "Error opening gnuplot" << std::endl;
        return;
    }

    // Generate the Gnuplot script
    fprintf(gnuplotPipe, "set term pngcairo\n");
    fprintf(gnuplotPipe, "set output '%s'\n", png_filename.c_str());
    fprintf(gnuplotPipe, "set xrange [%s:%s]\n", params.x_min.c_str(), params.x_max.c_str());
    fprintf(gnuplotPipe, "set yrange [%s:%s]\n", params.y_min.c_str(), params.y_max.c_str());
    fprintf(gnuplotPipe, "plot '%s%s%s' using 1:4 with lines title 'Data'\n", params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    // Get statistics on the data
    fprintf(gnuplotPipe, "stats [%s:%s][%s:%s] '%s%s%s' using ($1):($4) nooutput\n",
        params.x_min.c_str(), params.x_max.c_str(), params.y_min.c_str(), params.y_max.c_str(), params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    // Calculate y_63 and y_10
    fprintf(gnuplotPipe, "y_63 = STATS_min_y + (STATS_max_y - STATS_min_y) * 0.63\n");
    fprintf(gnuplotPipe, "y_10 = STATS_min_y + (STATS_max_y - STATS_min_y) * 0.1\n");

    // Find the x-values for y_63 and y_10
    fprintf(gnuplotPipe, "set print '%s' append\n", tmp_filename.c_str());
    fprintf(gnuplotPipe, "print 'x_63', STATS_index_min_x + (y_63 - STATS_min_y) / (STATS_max_y - STATS_min_y) * (STATS_index_max_x - STATS_index_min_x)\n");
    fprintf(gnuplotPipe, "print 'x_10', STATS_index_min_x + (y_10 - STATS_min_y) / (STATS_max_y - STATS_min_y) * (STATS_index_max_x - STATS_index_min_x)\n");

    // Close Gnuplot pipe
    fflush(gnuplotPipe);
    fprintf(gnuplotPipe, "exit\n");
    pclose(gnuplotPipe);

    // Output the results to the CSV file
    std::ifstream gnuplot_output(tmp_filename);
    if (gnuplot_output.is_open()) {
        std::string line;
        double x_63 = 0.0;
        double x_10 = 0.0;
        bool found_x63 = false;
        bool found_x10 = false;
        while (std::getline(gnuplot_output, line)) {
            std::stringstream ss(line);
            std::string key;
            double value;
            ss >> key >> value;
            if (key == "x_63") {
                x_63 = value;
                found_x63 = true;
            } else if (key == "x_10") {
                x_10 = value;
                found_x10 = true;
            }
        }
        if (found_x63 && found_x10) {
            output_csv << params.filename << "," << x_63 << "," << x_10 << "\n";
        } else {
            std::cerr << "Error: Did not find x_63 or x_10 in the Gnuplot output\n";
        }
        gnuplot_output.close();
    } else {
        std::cerr << "Error reading Gnuplot output file\n";
    }

    // Remove the temporary file
    std::remove(tmp_filename.c_str());
}

int main() {
    std::vector<FitParameters> parameter_sets = parse_csv("20240412.csv");

    // Open the output CSV file for the fitting parameters
    std::ofstream output_file("fitted_parameters.csv");
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open output CSV file." << std::endl;
        return 1;
    }
    // Write the header for the CSV file
    output_file << "filename,x_63,x_10\n";

    for (const auto& params : parameter_sets) {
        perform_curve_fit(params, output_file);
    }

    output_file.close();
    return 0;
}
