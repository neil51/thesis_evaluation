// Tried to output to csv

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>

struct FitParameters {
    std::string filename;
    std::string L;
    std::string D;
    std::string Dd;
    std::string x0;
    std::string xd0;
    std::string f0;
    std::string fd0;
    std::string finf;
    std::string fdinf;
    std::string xaxis_start;
    std::string xaxis_end;
    std::string yaxis_start;
    std::string yaxis_end;
    std::string start_fit_buildup;
    std::string end_fit_buildup;
    std::string start_fit_decay;
    std::string end_fit_decay;
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
        std::getline(ss, params.filename, ',');
        std::getline(ss, params.L, ',');
        std::getline(ss, params.D, ',');
        std::getline(ss, params.Dd, ',');
        std::getline(ss, params.x0, ',');
        std::getline(ss, params.xd0, ',');
        std::getline(ss, params.f0, ',');
        std::getline(ss, params.fd0, ',');
        std::getline(ss, params.finf, ',');
        std::getline(ss, params.fdinf, ',');
        std::getline(ss, params.xaxis_start, ',');
        std::getline(ss, params.xaxis_end, ',');
        std::getline(ss, params.yaxis_start, ',');
        std::getline(ss, params.yaxis_end, ',');
        std::getline(ss, params.start_fit_buildup, ',');
        std::getline(ss, params.end_fit_buildup, ',');
        std::getline(ss, params.start_fit_decay, ',');
        std::getline(ss, params.end_fit_decay, ',');

        parameter_sets.push_back(params);
    }

    return parameter_sets;
}

void perform_curve_fit(const FitParameters& params, std::ofstream& output_file) {
    FILE *gnuplotPipe = popen("gnuplot -persist", "w");
    if (!gnuplotPipe) {
        std::cerr << "Error opening gnuplot" << std::endl;
        return;
    }

    fprintf(gnuplotPipe, "set fit limit 1e-3 prescale\n");
    fprintf(gnuplotPipe, "set fit maxiter 20 \n");
    fprintf(gnuplotPipe, "set terminal qt \n");

    // Set output for the plots
    std::string output_png = params.filename + ".png";
    fprintf(gnuplotPipe, "set terminal pngcairo size 1920,1080\n");
    fprintf(gnuplotPipe, "set output '%s'\n", output_png.c_str());
    fprintf(gnuplotPipe, "set print '%s.log' \n", params.filename.c_str());

    fprintf(gnuplotPipe, "L=%s\n D=%s\n x0=%s\n f0=%s\n finf=%s\n",
            params.L.c_str(), params.D.c_str(), params.x0.c_str(), params.f0.c_str(), params.finf.c_str());

    fprintf(gnuplotPipe, "t(x)=D*(x-x0)/L**2 \n");
    fprintf(gnuplotPipe, "f(x)=2/sqrt(pi*t(x))*(exp(-1/(4*t(x))) + exp(-9/(4*t(x))) + exp(-25/(4*t(x))) + exp(-49/(4*t(x))))*finf + f0 \n");

    fprintf(gnuplotPipe, "Dd=%s\n xd0=%s\n fd0=%s\n fdinf=%s\n",
            params.Dd.c_str(), params.xd0.c_str(), params.fd0.c_str(), params.fdinf.c_str());

    fprintf(gnuplotPipe, "td(x)=Dd*(x-xd0)/L**2 \n");
    fprintf(gnuplotPipe, "g(x)=fdinf-2/sqrt(pi*td(x))*(exp(-1/(4*td(x))) + exp(-9/(4*td(x))) + exp(-25/(4*td(x))))*fdinf + fd0 \n");

    fprintf(gnuplotPipe, "print 'Fit buildup curve' \n");

    fprintf(gnuplotPipe, "fit [%s:%s] f(x) '%s' us ($1):($4) via D,f0,x0,finf\n",
            params.start_fit_buildup.c_str(), params.end_fit_buildup.c_str(), params.filename.c_str());
    
    fprintf(gnuplotPipe, "print D, f0, x0, finf \n");

    fprintf(gnuplotPipe, "print 'Fit decay curve' \n");

    fprintf(gnuplotPipe, "fit [%s:%s] g(x) '%s' us ($1):($4) via Dd,xd0,fdinf,fd0\n",
            params.start_fit_decay.c_str(), params.end_fit_decay.c_str(), params.filename.c_str());

    fprintf(gnuplotPipe, "plot [%s:%s][%s:%s] '%s' us ($1):($4), f(x), g(x)\n",
            params.xaxis_start.c_str(), params.xaxis_end.c_str(), params.yaxis_start.c_str(), params.yaxis_end.c_str(), params.filename.c_str());

    // Output the fitted parameters to the CSV file
    // output_file << params.filename << "," << params.xaxis_start << "," << params.xaxis_end << "," << D_fit << "," << D_err << "\n";

    fflush(gnuplotPipe);
    fprintf(gnuplotPipe, "exit \n");
    pclose(gnuplotPipe);

    // printf("%d",variable);
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
    output_file << "filename,xaxis_start,xaxis_end,D,D_err\n";

    for (const auto& params : parameter_sets) {
        perform_curve_fit(params, output_file);
    }

    output_file.close();
    return 0;
}
