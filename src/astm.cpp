// can calculate x_63, x_10 for buildup and decay
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
        if (!(iss >> d.t >> d.V >> d.I >> d.S)) {
            std::cerr << "Error parsing line: " << line << std::endl;
            continue; // Skip lines that cannot be parsed
        }
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
    fprintf(gnuplotPipe, "plot [%s:%s][%s:%s] '%s%s%s' us ($1):($4)\n",params.x_min.c_str(), params.x_max.c_str(), params.y_min.c_str(), params.y_max.c_str(), params.path.c_str(), params.filename.c_str(), params.suffix.c_str());

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

    // fprintf(gnuplotPipe, "print 'y_max = ', y_max\n print 'y_min = ', y_min\n print 'x(y_max) = ', x_pos_max_y\n print 'x(y_min) = ', x_pos_min_y\n print 'x_63,y_63 = ', x_63, y_63\n print 'x_10,y_10 = ', x_10,y_10\n");
    
    // fprintf(gnuplotPipe, "print y_coord_63, y_coord_10\n");

    fprintf(gnuplotPipe, "print y_coord_63, ',', y_coord_10, ',', x_pos_max_y, ',', x_pos_min_y \n");

    // fprintf(gnuplotPipe, "replot \n");

    fprintf(gnuplotPipe, "\n");

    // fprintf(gnuplotPipe, "print y_63\n");

    // Open the file
    std::ifstream infile(tmp_filename);
    if (!infile.is_open()) {
        std::cerr << "Error opening file: " << tmp_filename << std::endl;
    }

    double y_63, y_10, x_max_fit, x_min_fit;

    // Read the Gnuplot output
    std::string line;
    if (std::getline(infile, line)) {
        std::istringstream iss(line);
        char comma;
        if (!(iss >> y_63 >> comma >> y_10 >> comma >> x_max_fit >> comma >> x_min_fit)) {
            std::cerr << "Error parsing Gnuplot output: " << line << std::endl;
            return;
        }

        std::vector<Data> data = readData(params.path + params.filename + params.suffix);

        // Set the target value, tolerance, and limits
        double targetS_10 = y_10;
        double targetS_63 = y_63;
        double tolerance = 0.0005;
        double lowerLimit;
        double upperLimit;
        double t_63;
        double t_10;
        double L = 0.0004; // 
        double M_63 = 6.0;  // 
        double M_10 = 15.3; // 
        double x_63;
        double x_10;


        if (x_max_fit > x_min_fit){ // buildup
            lowerLimit = x_min_fit;
            upperLimit = x_max_fit;
            double x_10 = findValue(data, targetS_10, tolerance, lowerLimit, upperLimit);
            double x_63 = findValue(data, targetS_63, tolerance, lowerLimit, upperLimit);
            t_63 = x_63 - lowerLimit;
            t_10 = x_10 - lowerLimit;
        } else { // decay
            lowerLimit = x_max_fit;
            upperLimit = x_min_fit;
            double x_10 = findValue(data, targetS_10, tolerance, lowerLimit, upperLimit);
            double x_63 = findValue(data, targetS_63, tolerance, lowerLimit, upperLimit);
            t_63 = upperLimit - x_63;
            t_10 = upperLimit - x_10;
        }

        // Dapp_63
        double D_63 = (std::pow(L, 2)) / (M_63 * t_63);

        // Dapp_63
        double D_10 = (std::pow(L, 2)) / (M_10 * t_10);

        double D_avg = (D_63 + D_10) / 2;

        // Output the result

        std::cout << "xmin = " << lowerLimit << std::endl;
        std::cout << "xmax = " << upperLimit << std::endl;
        std::cout << "D_avg = " << D_avg << std::endl;
        // std::cout << "D_63 = " << D_63 << std::endl;
        // std::cout << "D_10 = " << D_10 << std::endl;

        // Append ".csv" extension to the filename
        std::string outputFilename = "astm_output/" + params.filename + "_astm.csv";

    // Open the CSV file in append mode
        std::ofstream file(outputFilename, std::ios::app);
    
    // // Check if the file is open
    //     if (!file.is_open()) {
    //         std::cerr << "Failed to open file: " << outputFilename << std::endl;
    //     }

    // Write the values to the CSV file
        // file << "xmin,xmax,D_avg\n"; // Write header if needed, remove if not
        file << lowerLimit << "," << upperLimit << "," << D_avg << "\n";

    // Close the file
        file.close();

        // std::cout << "Values written to " << outputFilename << std::endl;
        infile.close();

    // fprintf(gnuplotPipe, "set object circle at %f,y_coord_63 \n", x_63);
    // fprintf(gnuplotPipe, "set object circle at %f,y_coord_10 \n", x_10);
    std::remove(tmp_filename.c_str());

    // fprintf(gnuplotPipe, "replot \n");
    fflush(gnuplotPipe);
    fprintf(gnuplotPipe, "exit \n");
    pclose(gnuplotPipe);
    
    }

}

int main() {
    std::vector<FitParameters> parameter_sets = parse_csv("astm_input/20240612_astm_input.csv");
 
     for (const auto& params : parameter_sets) {
        perform_curve_fit(params);
    }

    return 0;
}
