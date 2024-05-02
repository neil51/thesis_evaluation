#include "../gnuplot.h"

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