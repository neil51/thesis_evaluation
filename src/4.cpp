// can print ymax, ymin, xmax, xmin
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

void perform_curve_fit(const FitParameters& params) {

    // Create a temporary file for Gnuplot output
    std::string tmp_filename = "gnuplot_output.tmp";
    FILE* gnuplotPipe = popen(("gnuplot -persist > " + tmp_filename).c_str(), "w");
    if (!gnuplotPipe) {
        std::cerr << "Error opening gnuplot" << std::endl;
        return;
    }
    fprintf(gnuplotPipe, "\n");
    fprintf(gnuplotPipe, "set terminal qt\n");
    fprintf(gnuplotPipe, "plot [%s:%s][%s:%s] '%s%s%s' us ($1):($4)\n",
            params.x_min.c_str(), params.x_max.c_str(), params.y_min.c_str(), params.y_max.c_str(), params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    fprintf(gnuplotPipe, "stats [%s:%s][%s:%s] '%s%s%s' us ($1):($4) nooutput\n",
            params.x_min.c_str(), params.x_max.c_str(), params.y_min.c_str(), params.y_max.c_str(), params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    fprintf(gnuplotPipe, "print GPVAL_DATA_Y_MAX\n");
    fprintf(gnuplotPipe, "print GPVAL_DATA_Y_MIN\n");

    fprintf(gnuplotPipe, "print STATS_pos_max_y\n");
    fprintf(gnuplotPipe, "print STATS_pos_min_y\n");

    fprintf(gnuplotPipe, "y_63 = (GPVAL_DATA_Y_MAX - GPVAL_DATA_Y_MIN) * 0.63 \n");
    fprintf(gnuplotPipe, "y_10 = (GPVAL_DATA_Y_MAX - GPVAL_DATA_Y_MIN) * 0.1 \n");

    fprintf(gnuplotPipe, "\n");

    // fprintf(gnuplotPipe, "print y_63\n");

    fflush(gnuplotPipe);
    fprintf(gnuplotPipe, "exit \n");
    pclose(gnuplotPipe);
}

int main() {
    std::vector<FitParameters> parameter_sets = parse_csv("astm.csv");
 
     for (const auto& params : parameter_sets) {
        perform_curve_fit(params);
    }

    return 0;
}
