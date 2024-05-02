#include <cmath>
#include <vector>
#include <stdexcept>
#include <numeric>
#include <algorithm> // for std::min
#include "../gnuplot.h"

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
double decay(int n, double L, double D, double t, double t1){
    double c = const_val(L, D, t, t1);
    std::vector<double> ser = series(n, L, D, t, t1);
    double sum = std::accumulate(ser.begin(), ser.end(), 0.0);
    return 1 - (c * sum);
}  

// Calculate current density buildup
std::vector<double> current_density_buildup(double D, const std::vector<double>& t, int n, double L, double jss, double j0, double t0){
    int len = t.size();
    std::vector<double> j(len);

    for (int i = 0; i < len; ++i){
        j[i] = (curve(n, L, D, t[i], t0) * (jss - j0)) + j0; 
    } 
    return j;
} 

// Calculate current density decay
std::vector<double> current_density_decay(double D, const std::vector<double>& t, int n, double L, double jss, double j0, double t0){
    int len = t.size();
    std::vector<double> j(len);

    for (int i = 0; i < len; ++i){
        j[i] = 1 - (curve(n, L, D, t[i], t0) * (jss - j0)) + j0; 
    } 
    return j;
} 



int main(){
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