#include <cmath>
// #include "/home/neil/dev/include/fmt/core.h"
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <numeric>
#include <algorithm> // for std::min
#include "/home/neil/dev/include/gnuplot.h"

using namespace std;

// Calculate the ratio
double ratio(double L, double D, double t, double t0){
    if (t - t0 == 0.0){
        return 0.0; // To prevent division by zero
    }
    return (L * L) / (4 * D * (t - t0));
}

// Calculate the constant value within the series
double const_val(double L, double D, double t, double t0){
    if (t - t0 == 0.0){
        return 0.0; // To prevent division by zero
    } 
    return (2 * L) / std::sqrt(M_PI * D * (t - t0));
} 

// Generate the array for the series term
std::vector<double> generate_array(int n){
    std::vector<double> array(n);
    for (int i = 0; i < n; ++i) {
        array[i] = -std::pow(2 * i + 1, 2);
    } 
    return array;
} 

// Define the series based on generated array
std::vector<double> series(int n, double L, double D, double t, double t0){
    std::vector<double> laplace = generate_array(n);
    std::vector<double> result(n);

    for (int i = 0; i < n; ++i){
        result[i] = std::exp(laplace[i] * ratio(L, D, t, t0));
    }  
    return result;
} 

// Define the buildup curve
double curve(int n, double L, double D, double t, double t0){
    double c = const_val(L, D, t, t0);
    std::vector<double> ser = series(n, L, D, t, t0);
    double sum = std::accumulate(ser.begin(), ser.end(), 0.0);
    return c * sum;
} 

// Define the decay curve
double decay_curve(int n, double L, double D, double t, double t1){
    double c = const_val(L, D, t, t1);
    std::vector<double> ser = series(n, L, D, t, t1);
    double sum = std::accumulate(ser.begin(), ser.end(), 0.0);
    return 1 - (c * sum);
}  

// Calculate current density buildup
std::vector<double> buildup(double D, const std::vector<double>& t, int n, double L, double jss, double j0, double t0){
    int len = t.size();
    std::vector<double> j(len);

    for (int i = 0; i < len; ++i){
        j[i] = (curve(n, L, D, t[i], t0) * (jss - j0)) + j0; 
    } 
    return j;
} 

// Calculate current density decay
std::vector<double> decay(double D, const std::vector<double>& t, int n, double L, double jss, double j0, double t0){
    int len = t.size();
    std::vector<double> j(len);

    for (int i = 0; i < len; ++i){
        j[i] = 1 - (curve(n, L, D, t[i], t0) * (jss - j0)) + j0; 
    } 
    return j;
} 

int plot(){
    GnuplotPipe gp;
    gp.sendLine("f(x)=2/sqrt(pi*t(x))*(exp(-1/(4*t(x)))-exp(-9/(4*t(x)))+exp(-25/(4*t(x))))*finf+f0");
    gp.sendLine("t(x)=D*(x-x0)/L**2");
    gp.sendLine("D=2e-12");
    gp.sendLine("x0=2000");
    gp.sendLine("finf=0.23");
    gp.sendLine("f0=0.005");
    gp.sendLine("L=0.0004");
    gp.sendLine("g(x)=fdinf-2/sqrt(pi*td(x))*(exp(-1/(4*td(x)))-exp(-9/(4*td(x)))+exp(-25/(4*td(x))))*fdinf+fd0");
    gp.sendLine("td(x)=Dd*(x-xd0)/L**2");
    gp.sendLine("fdinf=0.23");
    gp.sendLine("Dd=1e-11");
    gp.sendLine("fd0=0.05");
    gp.sendLine("xd0=9000");
    gp.sendLine("fit [4000:10000] f(x) '20240229_01.ASC' us ($1):($4) via D,x0,finf,f0");
    gp.sendLine("fit [12000:16000] g(x) '20240229_01.ASC' us ($1):($4) via Dd,xd0,fdinf,fd0");
    gp.sendLine("p [200:30000][0:0.25]'20240229_01.ASC' us ($1):($4),f(x),g(x)");
    gp.sendLine("print D, x0, f0, finf");
    gp.sendLine("print Dd, xd0, fd0, fdinf");
    return 0;
}

// Function to read data from a file and return it as a 2D vector
std::vector<std::vector<double>> readFile(const std::string& filename) {
    // Create a 2D vector to store the data
    std::vector<std::vector<double>> data;

    // Open the file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return data; // Return empty vector if the file can't be opened
    }

    std::string line;
    while (std::getline(file, line)) {
        // Create a stringstream to parse the line
        std::stringstream ss(line);

        // Vector to hold the values from the line
        std::vector<double> row;
        double value;

        // Read each value from the line
        while (ss >> value) {
            row.push_back(value);
        }

        // Add the row to the data vector
        data.push_back(row);
    }

    // Close the file
    file.close();

    return data; // Return the 2D vector
}

int main(){
    // Define the filename
    std::string filename = "../20240229/20240229_01.ASC"; // Change this to your file's path/name
    // GnuplotPipe gp;
    FILE *gnuplotPipe = popen("gnuplot -persist", "w");
    fprintf(gnuplotPipe,"\n");
    std::stringstream ss;

    std::string error_tolerance("1e-2");
    // int n = 5;      // number of terms for the mathematical series
    double L = 0.0004;  // thickness of the sample (in m)
    std::string D("1E-11");   // initial guess for D (buildup)
    std::string Dd("1E-11");  // initial guess for Dd (decay)
    double x0 = 2000;   // initial guess for x0 (start of buildup curve)
    double xd0 = 9000;  // initial guess for xd0 (start of decay curve)
    double f0 = 0.005;  // initial guess for f0 (residual current, buildup curve)
    double fd0 = 0.05;  // initial guess for fd0 (residual current, decay curve)
    double finf = 0.23; // initial guess for finf (steady state current, buildup curve)
    double fdinf = 0.23;    // initial guess for fdinf (steady state current, decay curve)
    double xaxis_start = 200; // start of x axis displayed
    double xaxis_end = 30000;    // end of x axis displayed
    double yaxis_start = 0;    // start of y axis displayed
    double yaxis_end = 0.25;   // end of y axis displayed
    int start_fit_buildup = 4000;   // start for buildup curve fit
    int end_fit_buildup = 10000;    // end for buildup curve fit
    int start_fit_decay = 12000;    // start for decay curve fit
    int end_fit_decay = 20000;      // end for decay curve fit


    // fprintf(gnuplotPipe, "set fit maxiter 20 prescale\n");    // set max iterations
    fprintf(gnuplotPipe, "set fit limit %s prescale\n", error_tolerance.c_str());      // set error value

    fprintf(gnuplotPipe, "L=%f\n D=%s\n x0=%f\n f0=%f\n finf=%f\n", L, D.c_str(), x0, f0, finf);    // Initial values for variables, and fixed terms

    fprintf(gnuplotPipe, "f(x)=2/sqrt(pi*t(x))*(exp(-1/(4*t(x)))+exp(-9/(4*t(x)))+exp(-25/(4*t(x)))+exp(-49/(4*t(x)))+exp(-81/(4*t(x)))+exp(-121/(4*t(x))))*finf+f0 \n");
    fprintf(gnuplotPipe, "t(x)=D*(x-x0)/L**2 \n");
    fprintf(gnuplotPipe, "g(x)=fdinf-2/sqrt(pi*td(x))*(exp(-1/(4*td(x)))+exp(-9/(4*td(x)))+exp(-25/(4*td(x)))+exp(-49/(4*td(x))))*fdinf+fd0 \n");
    fprintf(gnuplotPipe, "td(x)=Dd*(x-xd0)/L**2 \n");
    fprintf(gnuplotPipe, "Dd=%s\n xd0=%f\n fd0=%f\n fdinf=%f\n", Dd.c_str(), xd0, fd0, fdinf);    // Initial values for variables, and fixed terms



    fprintf(gnuplotPipe, "fit [%d:%d] f(x) '%s' us ($1):($4) via D,x0,finf,f0\n ", start_fit_buildup, end_fit_buildup, filename.c_str());
    fprintf(gnuplotPipe, "fit [%d:%d] g(x) '%s' us ($1):($4) via Dd,xd0,fdinf,fd0\n", start_fit_decay, end_fit_decay, filename.c_str());
    fprintf(gnuplotPipe, "p [%f:%f][%f:%f] '%s' us ($1):($4),f(x),g(x)\n", xaxis_start, xaxis_end, yaxis_start, yaxis_end, filename.c_str()); // Plot original curve

    // fprintf(gnuplotPipe, "print D, x0, f0, finf\n print D_err, x0_err, f0_err, finf_err \n");

    fflush(gnuplotPipe);
    fprintf(gnuplotPipe,"exit \n");     // exit gnuplot
    pclose(gnuplotPipe);
    



    // Read the data from the file
    // std::vector<std::vector<double>> data = readFile(filename);

    // Output the data to verify it was imported correctly
    // std::cout << "t  v  i  s" << std::endl;
    // for (const auto& row : data) {
    //     for (const auto& value : row) {
    //         std::cout << value << "  ";
    //     }
    //     std::cout << std::endl;
    // }

    // const int rows = data.size();
    // std::vector<double> time(rows); // Size is the same as the number of rows
    // std::vector<double> current_density(rows);

    // for (int i = 0; i < rows; ++i) { // Only starting from row 200 to reduce initial noise
    //     time[i] = data[i][0]; // Copy the first column of each row
    // }

    // for (int i = 0; i < rows; ++i) { // Only starting from row 200 to reduce initial noise
    //     current_density[i] = data[i][3]; // Copy the first column of each row
    // }

    // std::vector<double> f(rows);
    // f = buildup(D, time, n, L, finf, f0, x0);

    // // Save calculated values to my_file.txt
    // std::ofstream outFile("my_file.txt");
    // for (const auto &e : f) outFile << e << "\n";
    // // To check output
    // for (const auto& s : f) {
    //     std::cout << s << " ";
    // }
    // std::cout << std::endl;

    // ss << "p [200:30000][0:0.25]'" << filename << "' us ($1):($4)";
    // gp.sendLine(ss.str());

    return 0;
} 