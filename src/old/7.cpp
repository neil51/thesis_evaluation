#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <cmath>


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
    double y_min;
    double y_max;
    double x_min;
    double x_max;
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
        params.y_min = std::stod(params.y_min);
        params.y_max = std::stod(params.y_max);
        params.x_min = std::stod(params.x_min);
        params.x_max = std::stod(params.x_max);
        parameter_sets.push_back(params);
    }

    return parameter_sets;
}

double compute_y_value(const FitParameters& params, double x) {
    // Implement this function to return the y-value of the curve at x.
    // For example, if the curve is a polynomial or can be described by an equation, compute the y-value.
    // This is a placeholder implementation.
    // Replace this with the actual computation based on the data file or curve equation.
    return std::sin(x); // Example placeholder
}

double find_x_for_y(const FitParameters& params, double target_y, double tolerance = 1e-6) {
    double left = params.x_min;
    double right = params.x_max;
    double mid;

    while (right - left > tolerance) {
        mid = left + (right - left) / 2.0;
        double mid_y = compute_y_value(params, mid);

        if (std::fabs(mid_y - target_y) < tolerance) {
            return mid;
        } else if (mid_y < target_y) {
            left = mid;
        } else {
            right = mid;
        }
    }

    return mid; // Return the mid-point as the best approximation
}

void perform_curve_fit(const FitParameters& params) {
    double x_63 = find_x_for_y(params, (params.y_max - params.y_min) * 0.63 + params.y_min);
    double x_10 = find_x_for_y(params, (params.y_max - params.y_min) * 0.1 + params.y_min);

    std::cout << "For file: " << params.path << params.filename << params.suffix << std::endl;
    std::cout << "x_63 = " << x_63 << std::endl;
    std::cout << "x_10 = " << x_10 << std::endl;

    // Create a temporary file for Gnuplot output
    std::string tmp_filename = "gnuplot_output.tmp";
    FILE* gnuplotPipe = popen(("gnuplot -persist > " + tmp_filename).c_str(), "w");
    if (!gnuplotPipe) {
        std::cerr << "Error opening gnuplot" << std::endl;
        return;
    }

    fprintf(gnuplotPipe, "set terminal qt\n");
    fprintf(gnuplotPipe, "plot [%f:%f][%f:%f] '%s%s%s' us ($1):($4)\n",
            params.x_min, params.x_max, params.y_min, params.y_max, params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    fprintf(gnuplotPipe, "set object circle at %f,%f \n", x_10, (params.y_max - params.y_min) * 0.1 + params.y_min);
    fprintf(gnuplotPipe, "set object circle at %f,%f \n", x_63, (params.y_max - params.y_min) * 0.63 + params.y_min);

    fprintf(gnuplotPipe, "replot \n");
    fprintf(gnuplotPipe, "exit \n");

    fflush(gnuplotPipe);
    pclose(gnuplotPipe);
}

int main() {
    std::vector<FitParameters> parameter_sets = parse_csv("astm.csv");

    for (const auto& params : parameter_sets) {
        perform_curve_fit(params);
    }

    return 0;
}
