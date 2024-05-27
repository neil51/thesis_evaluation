// can print ymax, ymin, xmax, xmin
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
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

// Structure to hold the data
struct Data {
    double t;
    double V;
    double I;
    double S;
};

// Function to read data from file
std::vector<Data> readData(const std::string& filename) {
    std::vector<Data> data;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Data d;
        if (!(iss >> d.t >> d.V >> d.I >> d.S)) { break; }
        data.push_back(d);
    }
    return data;
}

// Function to find the column 1 value for a given column 4 value within tolerance
double findValue(const std::vector<Data>& data, double targetS, double tolerance, double lowerLimit, double upperLimit) {
    for (const auto& d : data) {
        if (d.t >= lowerLimit && d.t <= upperLimit && std::fabs(d.S - targetS) <= tolerance) {
            return d.t;
        }
    }
    return -1; // Return -1 if no match is found
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
    std::string tmp_filename = "gnuplot_output.csv";
    FILE* gnuplotPipe = popen(("gnuplot -persist > " + tmp_filename).c_str(), "w");
    if (!gnuplotPipe) {
        std::cerr << "Error opening gnuplot" << std::endl;
        return;
    }
    fprintf(gnuplotPipe, "\n");
    fprintf(gnuplotPipe, "set terminal qt\n");
    fprintf(gnuplotPipe, "set print '%s'\n", tmp_filename.c_str());
    fprintf(gnuplotPipe, "plot [%s:%s][%s:%s] '%s%s%s' us ($1):($4)\n",
            params.x_min.c_str(), params.x_max.c_str(), params.y_min.c_str(), params.y_max.c_str(), params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    fprintf(gnuplotPipe, "stats [%s:%s][%s:%s] '%s%s%s' us ($1):($4) nooutput\n",
            params.x_min.c_str(), params.x_max.c_str(), params.y_min.c_str(), params.y_max.c_str(), params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

    fprintf(gnuplotPipe, "y_max = GPVAL_DATA_Y_MAX\n");   // max y axis value
    fprintf(gnuplotPipe, "y_min = GPVAL_DATA_Y_MIN\n");   // min y axis value

    fprintf(gnuplotPipe, "x_pos_max_y = STATS_pos_max_y\n");    // x axis value where y is max
    fprintf(gnuplotPipe, "x_pos_min_y = STATS_pos_min_y\n");    // x axis value where y is min

    fprintf(gnuplotPipe, "y_63 = (y_max - y_min) * 0.63 \n");   // absolute y_63
    fprintf(gnuplotPipe, "y_10 = (y_max - y_min) * 0.1 \n");    // absolute y_10

    fprintf(gnuplotPipe, "x_63 = x_pos_min_y + ((y_63 - y_min) / (y_max - y_min)) * (x_pos_max_y - x_pos_min_y)\n");
    fprintf(gnuplotPipe, "x_10 = x_pos_min_y + ((y_10 - y_min) / (y_max - y_min)) * (x_pos_max_y - x_pos_min_y)\n");

    fprintf(gnuplotPipe, "y_coord_63 = y_63 + y_min\n");    // y coordinate of y_63
    fprintf(gnuplotPipe, "y_coord_10 = y_10 + y_min\n");    // y coordinate of y_10

    // fprintf(gnuplotPipe, "x_coord_63 = x_63 + x_pos_min_y\n");    // x coordinate of x_63
    // fprintf(gnuplotPipe, "x_coord_10 = x_10 + x_pos_min_y\n");    // x coordinate of x_10

    // fprintf(gnuplotPipe, "set object circle at x_10,y_coord_10 \n");
    // fprintf(gnuplotPipe, "set object circle at x_63,y_coord_63 \n");

    // fprintf(gnuplotPipe, "print 'y_max = ', y_max\n print 'y_min = ', y_min\n print 'x(y_max) = ', x_pos_max_y\n print 'x(y_min) = ', x_pos_min_y\n print 'x_63,y_63 = ', x_63, y_63\n print 'x_10,y_10 = ', x_10,y_10\n");
    
    // fprintf(gnuplotPipe, "print y_coord_63, y_coord_10\n");

    fprintf(gnuplotPipe, "print y_coord_63, ',', y_coord_10, ',', 'x_pos_max_y', ',', 'x_pos_min_y' \n");

    // fprintf(gnuplotPipe, "set arrow from y_63,0 to y_63,1 nohead lc rgb \'red\' \n");
    fprintf(gnuplotPipe, "replot \n");

    fprintf(gnuplotPipe, "\n");

    // fprintf(gnuplotPipe, "print y_63\n");

    fflush(gnuplotPipe);
    fprintf(gnuplotPipe, "exit \n");
    pclose(gnuplotPipe);

    // Open the file
    std::ifstream infile(tmp_filename);
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << tmp_filename << std::endl;
    }

    // Read the Gnuplot output
    std::string line;
    if (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string y_63_str, y_10_str, x_max_fit_str, x_min_fit_str;

        if (std::getline(iss, y_63_str, ',') && std::getline(iss, y_10_str, ',') && std::getline(iss, x_max_fit_str, ',') && std::getline(iss, x_min_fit_str, ',')) {
            std::vector<Data> data = readData("astm.csv");
            double y_63 = std::stod(y_63_str);
            double y_10 = std::stod(y_10_str);
            double x_max_fit = std::stod(x_max_fit_str);
            double x_min_fit = std::stod(x_min_fit_str);

            // Set the target value, tolerance, and limits

            double targetS = y_63;
            double tolerance = 0.0005;
            double lowerLimit = x_max_fit;
            double upperLimit = x_min_fit;

            // Find the value
            double result = findValue(data, targetS, tolerance, lowerLimit, upperLimit);
            // Print the result
            if (result != -1) {
                std::cout << "The value in column 1 for the given column 4 value is: " << result << std::endl;
            } else {
                std::cout << "No value found within the specified tolerance and limits." << std::endl;
            }
            
        // std::cout << "y_63 = " << y_63 << ", y_10 = " << y_10 << std::endl;
        } else {
            std::cerr << "Error parsing line: " << line << std::endl;
        }
    } else {
        std::cerr << "Error reading from file" << std::endl;
    }




    infile.close();
    std::remove(tmp_filename.c_str());

}

int main() {
    std::vector<FitParameters> parameter_sets = parse_csv("astm.csv");
 
     for (const auto& params : parameter_sets) {
        perform_curve_fit(params);
    }

    return 0;
}
