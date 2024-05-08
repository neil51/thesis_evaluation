#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdio>

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

void perform_curve_fit(const FitParameters& params) {
    FILE *gnuplotPipe = popen("gnuplot -persist", "w");
    if (!gnuplotPipe) {
        std::cerr << "Error opening gnuplot" << std::endl;
        return;
    }

    fprintf(gnuplotPipe, "set fit limit 1e-2 prescale\n");

    fprintf(gnuplotPipe, "L=%s\n D=%s\n x0=%s\n f0=%s\n finf=%s\n",
            params.L.c_str(), params.D.c_str(), params.x0.c_str(), params.f0.c_str(), params.finf.c_str());

    fprintf(gnuplotPipe, "t(x)=D*(x-x0)/L**2 \n");
    fprintf(gnuplotPipe, "f(x)=2/sqrt(pi*t(x))*(exp(-1/(4*t(x))) + exp(-9/(4*t(x))) + exp(-25/(4*t(x))) + exp(-49/(4*t(x))))*finf + f0 \n");

    fprintf(gnuplotPipe, "Dd=%s\n xd0=%s\n fd0=%s\n fdinf=%s\n",
            params.Dd.c_str(), params.xd0.c_str(), params.fd0.c_str(), params.fdinf.c_str());

    fprintf(gnuplotPipe, "td(x)=Dd*(x-xd0)/L**2 \n");
    fprintf(gnuplotPipe, "g(x)=fdinf-2/sqrt(pi*td(x))*(exp(-1/(4*td(x))) + exp(-9/(4*td(x))) + exp(-25/(4*td(x))))*fdinf + fd0 \n");

    fprintf(gnuplotPipe, "fit [%s:%s] f(x) '%s' us ($1):($4) via D,x0,finf,f0\n",
            params.start_fit_buildup.c_str(), params.end_fit_buildup.c_str(), params.filename.c_str());

    fprintf(gnuplotPipe, "fit [%s:%s] g(x) '%s' us ($1):($4) via Dd,xd0,fdinf,fd0\n",
            params.start_fit_decay.c_str(), params.end_fit_decay.c_str(), params.filename.c_str());

    fprintf(gnuplotPipe, "plot [%s:%s][%s:%s] '%s' us ($1):($4), f(x), g(x)\n",
            params.xaxis_start.c_str(), params.xaxis_end.c_str(), params.yaxis_start.c_str(), params.yaxis_end.c_str(), params.filename.c_str());

    fflush(gnuplotPipe);
    fprintf(gnuplotPipe, "exit \n");
    pclose(gnuplotPipe);
}

int main() {
    std::vector<FitParameters> parameter_sets = parse_csv("config.csv");

    for (const auto& params : parameter_sets) {
        perform_curve_fit(params);
    }

    return 0;
}
