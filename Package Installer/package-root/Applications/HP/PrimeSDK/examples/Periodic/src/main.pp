@disregard
 Copyright © 2024 Insoft. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
@end;

#include <pplang>
#include <pixel>
#include <dictionary>

local Periodic.table := {
#PPL
    {1,1,"H",1,1,"Hydrogen",1.00794,0.0899_g/1_cm^3,13.81_K,20.28_K,2.08_Å,14.304_J/(1_g*1_K),"1","1s1",2.1,14.1_cm^3/1_mol,13.598,0.1815_W/(1_m*1_K)},
    {18,1,"He",5,2,"Helium",4.0026,0.1785_g/1_cm^3,0.95_K,4.216_K,"n/a",5.193_J/(1_g*1_K),"n/a","1s2",0,31.8_cm^3/1_mol,24.587,0.152_W/(1_m*1_K)},
    {1,2,"Li",2,3,"Lithium",6.941,0.53_g/1_cm^3,453.7_K,1615_K,1.55_Å,3.582_J/(1_g*1_K),"1","1s2 2s1",0.98,13.1_cm^3/1_mol,5.392,84.7_W/(1_m*1_K)},
    {2,2,"Be",3,4,"Beryllium",9.01218,1.85_g/1_cm^3,1560_K,3243_K,1.12_Å,1.825_J/(1_g*1_K),"2","1s2 2s2",1.57,5_cm^3/1_mol,9.322,200_W/(1_m*1_K)},
    {13,2,"B",6,5,"Boron",10.811,2.34_g/1_cm^3,2365_K,4275_K,0.98_Å,1.026_J/(1_g*1_K),"3","1s2 2s2 p1",2.04,4.6_cm^3/1_mol,8.298,27_W/(1_m*1_K)},
    {14,2,"C",1,6,"Carbon",12.011,2.26_g/1_cm^3,3825_K,5100_K,0.91_Å,0.709_J/(1_g*1_K),"+/-4, 2","1s2 2s2 p2",2.55,5.3_cm^3/1_mol,11.26,155_W/(1_m*1_K)},
    {15,2,"N",1,7,"Nitrogen",14.0067,1.251_g/1_cm^3,63.15_K,77.344_K,0.92_Å,1.042_J/(1_g*1_K),"+/-3, 5, 4, 2","1s2 2s2 p3",3.04,17.3_cm^3/1_mol,14.534,0.02598_W/(1_m*1_K)},
    {16,2,"O",1,8,"Oxygen",15.9994,1.429_g/1_cm^3,54.8_K,90.188_K,0.65_Å,0.92_J/(1_g*1_K),"-2","1s2 2s2 p4",3.44,14_cm^3/1_mol,13.618,0.2674_W/(1_m*1_K)},
    {17,2,"F",4,9,"Fluorine",18.9984,1.696_g/1_cm^3,53.55_K,85_K,0.57_Å,0.824_J/(1_g*1_K),"-1","1s2 2s2 p5",3.98,17.1_cm^3/1_mol,17.422,0.0279_W/(1_m*1_K)},
    {18,2,"Ne",5,10,"Neon",20.1797,0.900_g/1_cm^3,24.55_K,27.1_K,0.51_Å,1.03_J/(1_g*1_K),"n/a","1s2 2s2 p6",0,16.9_cm^3/1_mol,21.564,0.0493_W/(1_m*1_K)},
    {1,3,"Na",2,11,"Sodium",22.98977,0.97_g/1_cm^3,371_K,1156_K,1.9_Å,1.23_J/(1_g*1_K),"1","[Ne] 3s1",0.93,23.7_cm^3/1_mol,5.139,141_W/(1_m*1_K)},
    {2,3,"Mg",3,12,"Magnesium",24.305,1.74_g/1_cm^3,922_K,1380_K,1.6_Å,1.02_J/(1_g*1_K),"2","[Ne] 3s2",1.31,14_cm^3/1_mol,7.646,156_W/(1_m*1_K)},
    {13,3,"Al",7,13,"Aluminum",26.98154,2.7_g/1_cm^3,933.5_K,2740_K,1.43_Å,0.9_J/(1_g*1_K),"3","[Ne] 3s2 p1",1.61,10_cm^3/1_mol,5.986,237_W/(1_m*1_K)},
    {14,3,"Si",6,14,"Silicon",28.0855,2.33_g/1_cm^3,1683_K,2630_K,1.32_Å,0.70_J/(1_g*1_K),"4","[Ne] 3s2 p2",1.9,12.1_cm^3/1_mol,8.151,148_W/(1_m*1_K)},
    {15,3,"P",1,15,"Phosphorus",30.97376,1.82_g/1_cm^3,317.3_K,553_K,1.28_Å,0.769_J/(1_g*1_K),"+/-3, 5, 4","[Ne] 3s2 p3",2.19,17_cm^3/1_mol,10.486,0.235_W/(1_m*1_K)},
    {16,3,"S",1,16,"Sulfur",32.066,2.07_g/1_cm^3,392.2_K,717.82_K,1.27_Å,0.71_J/(1_g*1_K),"+/-2, 4, 6","[Ne] 3s2 p4",2.58,15.5_cm^3/1_mol,10.36,0.269_W/(1_m*1_K)},
    {17,3,"Cl",4,17,"Chlorine",35.4527,3.214_g/1_cm^3,172.17_K,239.18_K,0.97_Å,0.48_J/(1_g*1_K),"+/-1, 3, 5, 7","[Ne] 3s2 p5",3.16,18.7_cm^3/1_mol,12.967,0.0089_W/(1_m*1_K)},
    {18,3,"Ar",5,18,"Argon",39.948,1.784_g/1_cm^3,83.95_K,87.45_K,0.88_Å,0.52_J/(1_g*1_K),"n/a","[Ne] 3s2 p6",0,24.2_cm^3/1_mol,15.759,0.0177_W/(1_m*1_K)},
    {1,4,"K",2,19,"Potassium",39.0983,0.86_g/1_cm^3,336.8_K,1033_K,2.35_Å,0.757_J/(1_g*1_K),"1","[Ar] 4s1",0.82,45.3_cm^3/1_mol,4.341,102.5_W/(1_m*1_K)},
    {2,4,"Ca",3,20,"Calcium",40.078,1.55_g/1_cm^3,1112_K,1757_K,1.97_Å,0.647_J/(1_g*1_K),"2","[Ar] 4s2",1,29.9_cm^3/1_mol,6.113,200_W/(1_m*1_K)},
    {3,4,"Sc",8,21,"Scandium",44.9559,2.99_g/1_cm^3,1814_K,3109_K,1.62_Å,0.568_J/(1_g*1_K),"3","[Ar] 3d1 4s2",1.36,15_cm^3/1_mol,6.54,15.8_W/(1_m*1_K)},
    {4,4,"Ti",8,22,"Titanium",47.88,4.54_g/1_cm^3,1945_K,3560_K,1.45_Å,0.523_J/(1_g*1_K),"4, 3","[Ar] 3d2 4s2",1.54,10.6_cm^3/1_mol,6.82,21.9_W/(1_m*1_K)},
    {5,4,"V",8,23,"Vanadium",50.9415,6.11_g/1_cm^3,2163_K,3650_K,1.34_Å,0.489_J/(1_g*1_K),"5, 4, 3, 2","[Ar] 3d3 4s2",1.63,8.35_cm^3/1_mol,6.74,30.7_W/(1_m*1_K)},
    {6,4,"Cr",8,24,"Chromium",51.996,7.19_g/1_cm^3,2130_K,2945_K,1.3_Å,0.449_J/(1_g*1_K),"6, 3, 2","[Ar] 3d5 4s1",1.66,7.23_cm^3/1_mol,6.766,93.7_W/(1_m*1_K)},
    {7,4,"Mn",8,25,"Manganese",54.938,7.44_g/1_cm^3,1518_K,2335_K,1.35_Å,0.48_J/(1_g*1_K),"7, 6, 4, 2, 3","[Ar] 3d5 4s2",1.55,7.39_cm^3/1_mol,7.435,7.82_W/(1_m*1_K)},
    {8,4,"Fe",8,26,"Iron",55.847,7.874_g/1_cm^3,1808_K,3023_K,1.26_Å,0.449_J/(1_g*1_K),"2, 3","[Ar] 3d6 4s2",1.83,7.1_cm^3/1_mol,7.87,80.2_W/(1_m*1_K)},
    {9,4,"Co",8,27,"Cobalt",58.9332,8.9_g/1_cm^3,1768_K,3143_K,1.25_Å,0.421_J/(1_g*1_K),"2, 3","[Ar] 3d7 4s2",1.88,6.7_cm^3/1_mol,7.86,100_W/(1_m*1_K)},
    {10,4,"Ni",8,28,"Nickel",58.6934,8.9_g/1_cm^3,1726_K,3005_K,1.24_Å,0.444_J/(1_g*1_K),"2, 3","[Ar] 3d8 4s2",1.91,6.6_cm^3/1_mol,7.635,90.7_W/(1_m*1_K)},
    {11,4,"Cu",8,29,"Copper",63.546,8.96_g/1_cm^3,1356.6_K,2840_K,1.28_Å,0.385_J/(1_g*1_K),"2, 1","[Ar] 3d10 4s1",1.9,7.1_cm^3/1_mol,7.726,401_W/(1_m*1_K)},
    {12,4,"Zn",8,30,"Zinc",65.39,7.13_g/1_cm^3,692.73_K,1180_K,1.38_Å,0.388_J/(1_g*1_K),"2","[Ar] 3d10 4s2",1.65,9.2_cm^3/1_mol,9.394,116_W/(1_m*1_K)},
    {13,4,"Ga",7,31,"Gallium",69.723,5.91_g/1_cm^3,302.92_K,2478_K,1.41_Å,0.371_J/(1_g*1_K),"3","[Ar] 3d10 4s2 p1",1.81,11.8_cm^3/1_mol,5.999,40.6_W/(1_m*1_K)},
    {14,4,"Ge",6,32,"Germanium",72.61,5.32_g/1_cm^3,1211.5_K,3107_K,1.37_Å,0.32_J/(1_g*1_K),"4","[Ar] 3d10 4s2 p2",2.01,13.6_cm^3/1_mol,7.899,59.9_W/(1_m*1_K)},
    {15,4,"As",6,33,"Arsenic",74.9216,5.78_g/1_cm^3,1090_K,876_K,1.39_Å,0.33_J/(1_g*1_K),"+/-3, 5","[Ar] 3d10 4s2 p3",2.18,13.1_cm^3/1_mol,9.81,50_W/(1_m*1_K)},
    {16,4,"Se",1,34,"Selenium",78.96,4.79_g/1_cm^3,494_K,958_K,1.4_Å,0.32_J/(1_g*1_K),"-2, 4, 6","[Ar] 3d10 4s2 p4",2.55,16.5_cm^3/1_mol,9.752,2.04_W/(1_m*1_K)},
    {17,4,"Br",4,35,"Bromine",79.904,3.12_g/1_cm^3,265.95_K,331.85_K,1.12_Å,0.226_J/(1_g*1_K),"+/-1, 5","[Ar] 3d10 4s2 p5",2.96,23.5_cm^3/1_mol,11.814,0.122_W/(1_m*1_K)},
    {18,4,"Kr",5,36,"Krypton",83.8,3.75_g/1_cm^3,116_K,120.85_K,1.03_Å,0.248_J/(1_g*1_K),"n/a","[Ar] 3d10 4s2 p6",0,32.2_cm^3/1_mol,13.999,0.00949_W/(1_m*1_K)},
    {1,5,"Rb",2,37,"Rubidium",85.4678,1.532_g/1_cm^3,312.63_K,961_K,2.48_Å,0.363_J/(1_g*1_K),"1","[Kr] 5s1",0.82,55.9_cm^3/1_mol,4.177,58.2_W/(1_m*1_K)},
    {2,5,"Sr",3,38,"Strontium",87.62,2.54_g/1_cm^3,1042_K,1655_K,2.15_Å,0.3_J/(1_g*1_K),"2","[Kr] 5s2",0.95,33.7_cm^3/1_mol,5.695,35.3_W/(1_m*1_K)},
    {3,5,"Y",8,39,"Yttrium",88.9059,4.47_g/1_cm^3,1795_K,3611_K,1.78_Å,0.3_J/(1_g*1_K),"3","[Kr] 4d1 5s2",1.22,19.8_cm^3/1_mol,6.38,17.2_W/(1_m*1_K)},
    {4,5,"Zr",8,40,"Zirconium",91.224,6.51_g/1_cm^3,2128_K,4682_K,1.6_Å,0.278_J/(1_g*1_K),"4","[Kr] 4d2 5s2",1.33,14.1_cm^3/1_mol,6.84,22.7_W/(1_m*1_K)},
    {5,5,"Nb",8,41,"Niobium",92.9064,8.57_g/1_cm^3,2742_K,5015_K,1.46_Å,0.265_J/(1_g*1_K),"5, 3","[Kr] 4d4 5s1",1.6,10.8_cm^3/1_mol,6.88,53.7_W/(1_m*1_K)},
    {6,5,"Mo",8,42,"Molybdenum",95.94,10.22_g/1_cm^3,2896_K,4912_K,1.39_Å,0.25_J/(1_g*1_K),"6, 5, 4, 3, 2","[Kr] 4d5 5s1",2.16,9.4_cm^3/1_mol,7.099,138_W/(1_m*1_K)},
    {7,5,"Tc",8,43,"Technetium",98,11.5_g/1_cm^3,2477_K,4538_K,1.36_Å,0.24_J/(1_g*1_K),"7","[Kr] 4d5 5s2",1.9,8.5_cm^3/1_mol,7.28,50.6_W/(1_m*1_K)},
    {8,5,"Ru",8,44,"Ruthenium",101.07,12.37_g/1_cm^3,2610_K,4425_K,1.34_Å,0.238_J/(1_g*1_K),"2, 3, 4, 6, 8","[Kr] 4d7 5s1",2.2,8.3_cm^3/1_mol,7.37,117_W/(1_m*1_K)},
    {9,5,"Rh",8,45,"Rhodium",102.9055,12.41_g/1_cm^3,2236_K,3970_K,1.34_Å,0.242_J/(1_g*1_K),"2, 3, 4","[Kr] 4d8 5s1",2.28,8.3_cm^3/1_mol,7.46,150_W/(1_m*1_K)},
    {10,5,"Pd",8,46,"Palladium",106.42,12_g/1_cm^3,1825_K,3240_K,1.37_Å,0.244_J/(1_g*1_K),"2, 4","[Kr] 4d10",2.2,8.9_cm^3/1_mol,8.34,71.8_W/(1_m*1_K)},
    {11,5,"Ag",8,47,"Silver",107.868,10.5_g/1_cm^3,1235.08_K,2436_K,1.44_Å,0.232_J/(1_g*1_K),"1","[Kr] 4d10 5s1",1.93,10.3_cm^3/1_mol,7.576,429_W/(1_m*1_K)},
    {12,5,"Cd",8,48,"Cadmium",112.41,8.65_g/1_cm^3,594.26_K,1040_K,1.71_Å,0.233_J/(1_g*1_K),"2","[Kr] 4d10 5s2",1.69,13.1_cm^3/1_mol,8.993,96.8_W/(1_m*1_K)},
    {13,5,"In",7,49,"Indium",114.82,7.31_g/1_cm^3,429.78_K,2350_K,1.66_Å,0.233_J/(1_g*1_K),"3","[Kr] 4d10 5s2 p1",1.78,15.7_cm^3/1_mol,5.786,81.6_W/(1_m*1_K)},
    {14,5,"Sn",7,50,"Tin",118.71,7.31_g/1_cm^3,505.12_K,2876_K,1.62_Å,0.228_J/(1_g*1_K),"4, 2","[Kr] 4d10 5s2 p2",1.96,16.3_cm^3/1_mol,7.344,66.6_W/(1_m*1_K)},
    {15,5,"Sb",6,51,"Antimony",121.757,6.69_g/1_cm^3,903.91_K,1860_K,1.59_Å,0.207_J/(1_g*1_K),"+/-3, 5","[Kr] 4d10 5s2 p3",2.05,18.4_cm^3/1_mol,8.641,24.3_W/(1_m*1_K)},
    {16,5,"Te",6,52,"Tellurium",127.6,6.24_g/1_cm^3,722.72_K,1261_K,1.42_Å,0.202_J/(1_g*1_K),"-2, 4, 6","[Kr] 4d10 5s2 p4",2.1,20.5_cm^3/1_mol,9.009,2.35_W/(1_m*1_K)},
    {17,5,"I",4,53,"Iodine",126.9045,4.93_g/1_cm^3,386.7_K,457.5_K,1.32_Å,0.145_J/(1_g*1_K),"+/-1, 5, 7","[Kr] 4d10 5s2 p5",2.66,25.7_cm^3/1_mol,10.451,0.449_W/(1_m*1_K)},
    {18,5,"Xe",5,54,"Xenon",131.29,5.9_g/1_cm^3,161.39_K,165.1_K,1.24_Å,0.158_J/(1_g*1_K),"n/a","[Kr] 4d10 5s2 p6",2.6,42.9_cm^3/1_mol,12.13,0.00569_W/(1_m*1_K)},
    {1,6,"Cs",2,55,"Cesium",132.9054,1.87_g/1_cm^3,301.54_K,944_K,2.67_Å,0.24_J/(1_g*1_K),"1","[Xe] 6s1",0.79,70_cm^3/1_mol,3.894,35.9_W/(1_m*1_K)},
    {2,6,"Ba",3,56,"Barium",137.33,3.59_g/1_cm^3,1002_K,2078_K,2.22_Å,0.204_J/(1_g*1_K),"2","[Xe] 6s2",0.89,39_cm^3/1_mol,5.212,18.4_W/(1_m*1_K)},
    {4,9,"La",9,57,"Lanthanum",138.9055,6.15_g/1_cm^3,1191_K,3737_K,1.38_Å,0.19_J/(1_g*1_K),"3","[Xe] 5d1 6s2",1.1,22.5_cm^3/1_mol,5.58,13.5_W/(1_m*1_K)},
    {5,9,"Ce",9,58,"Cerium",140.12,6.77_g/1_cm^3,1071_K,3715_K,1.81_Å,0.19_J/(1_g*1_K),"3, 4","[Xe] 4f1 5d1 6s2",1.12,21_cm^3/1_mol,5.47,11.4_W/(1_m*1_K)},
    {6,9,"Pr",9,59,"Praseodymium",140.9077,6.77_g/1_cm^3,1204_K,3785_K,1.82_Å,0.193_J/(1_g*1_K),"3, 4","[Xe] 4f3 6s2",1.13,20.8_cm^3/1_mol,5.42,12.5_W/(1_m*1_K)},
    {7,9,"Nd",9,60,"Neodymium",144.24,7.01_g/1_cm^3,1294_K,3347_K,1.82_Å,0.19_J/(1_g*1_K),"3","[Xe] 4f4 6s2",1.14,20.6_cm^3/1_mol,5.49,16.5_W/(1_m*1_K)},
    {8,9,"Pm",9,61,"Promethium",145,7.22_g/1_cm^3,1315_K,3273_K,"n/a","n/a","3","[Xe] 4f5 6s2",1.13,22.4_cm^3/1_mol,5.55,17.9_W/(1_m*1_K)},
    {9,9,"Sm",9,62,"Samarium",150.36,7.52_g/1_cm^3,1347_K,2067_K,1.81_Å,0.197_J/(1_g*1_K),"3, 2","[Xe] 4f6 6s2",1.17,19.9_cm^3/1_mol,5.63,13.3_W/(1_m*1_K)},
    {10,9,"Eu",9,63,"Europium",151.965,5.24_g/1_cm^3,1095_K,1800_K,1.99_Å,0.182_J/(1_g*1_K),"3, 2","[Xe] 4f7 6s2",1.2,28.9_cm^3/1_mol,5.67,13.9_W/(1_m*1_K)},
    {11,9,"Gd",9,64,"Gadolinium",157.25,7.9_g/1_cm^3,1585_K,3545_K,1.8_Å,0.236_J/(1_g*1_K),"3","[Xe] 4f7 5d1 6s2",1.2,19.9_cm^3/1_mol,6.15,10.6_W/(1_m*1_K)},
    {12,9,"Tb",9,65,"Terbium",158.9253,8.23_g/1_cm^3,1629_K,3500_K,1.8_Å,0.18_J/(1_g*1_K),"3, 4","[Xe] 4f9 6s2",1.1,19.2_cm^3/1_mol,5.86,11.1_W/(1_m*1_K)},
    {13,9,"Dy",9,66,"Dysprosium",162.5,8.55_g/1_cm^3,1685_K,2840_K,1.8_Å,0.173_J/(1_g*1_K),"3","[Xe] 4f10 6s2",1.22,19_cm^3/1_mol,5.93,10.7_W/(1_m*1_K)},
    {14,9,"Ho",9,67,"Holmium",164.9303,8.8_g/1_cm^3,1747_K,2968_K,1.79_Å,0.165_J/(1_g*1_K),"3","[Xe] 4f11 6s2",1.23,18.7_cm^3/1_mol,6.02,16.2_W/(1_m*1_K)},
    {15,9,"Er",9,68,"Erbium",167.26,9.07_g/1_cm^3,1802_K,3140_K,1.78_Å,0.168_J/(1_g*1_K),"3","[Xe] 4f12 6s2",1.24,18.4_cm^3/1_mol,6.101,14.3_W/(1_m*1_K)},
    {16,9,"Tm",9,69,"Thulium",168.9342,9.32_g/1_cm^3,1818_K,2223_K,1.77_Å,0.16_J/(1_g*1_K),"3, 2","[Xe] 4f13 6s2",1.25,18.1_cm^3/1_mol,6.184,16.8_W/(1_m*1_K)},
    {17,9,"Yb",9,70,"Ytterbium",173.04,6.97_g/1_cm^3,1092_K,1469_K,1.94_Å,0.155_J/(1_g*1_K),"3, 2","[Xe] 4f14 6s2",1.1,24.8_cm^3/1_mol,6.254,34.9_W/(1_m*1_K)},
    {18,9,"Lu",9,71,"Lutetium",174.967,9.84_g/1_cm^3,1936_K,3668_K,1.75_Å,0.15_J/(1_g*1_K),"3","[Xe] 4f14 5d1 6s2",1.27,17.8_cm^3/1_mol,5.43,16.4_W/(1_m*1_K)},
    {4,6,"Hf",8,72,"Hafnium",178.49,13.31_g/1_cm^3,2504_K,4875_K,1.67_Å,0.14_J/(1_g*1_K),"4","[Xe] 4f14 5d2 6s2",1.3,13.6_cm^3/1_mol,6.65,23_W/(1_m*1_K)},
    {5,6,"Ta",8,73,"Tantalum",180.9479,16.65_g/1_cm^3,3293_K,5730_K,1.49_Å,0.14_J/(1_g*1_K),"5","[Xe] 4f14 5d3 6s2",1.5,10.9_cm^3/1_mol,7.89,57.5_W/(1_m*1_K)},
    {6,6,"W",8,74,"Tungsten",183.85,19.3_g/1_cm^3,3695_K,5825_K,1.41_Å,0.13_J/(1_g*1_K),"6, 5, 4, 3, 2","[Xe] 4f14 5d4 6s2",2.36,9.53_cm^3/1_mol,7.98,174_W/(1_m*1_K)},
    {7,6,"Re",8,75,"Rhenium",186.207,21_g/1_cm^3,3455_K,5870_K,1.37_Å,0.137_J/(1_g*1_K),"7, 6, 4, 2, -1","[Xe] 4f14 5d5 6s2",1.9,8.85_cm^3/1_mol,7.88,47.9_W/(1_m*1_K)},
    {8,6,"Os",8,76,"Osmium",190.2,22.6_g/1_cm^3,3300_K,5300_K,1.35_Å,0.13_J/(1_g*1_K),"2, 3, 4, 6, 8","[Xe] 4f14 5d6 6s2",2.2,8.43_cm^3/1_mol,8.7,87.6_W/(1_m*1_K)},
    {9,6,"Ir",8,77,"Iridium",192.22,22.6_g/1_cm^3,2720_K,4700_K,1.36_Å,0.13_J/(1_g*1_K),"2, 3, 4, 6","[Xe] 4f14 5d7 6s2",2.2,8.54_cm^3/1_mol,9.1,147_W/(1_m*1_K)},
    {10,6,"Pt",8,78,"Platinum",195.08,21.45_g/1_cm^3,2042.1_K,4100_K,1.39_Å,0.13_J/(1_g*1_K),"2, 4","[Xe] 4f14 5d9 6s1",2.28,9.1_cm^3/1_mol,9,71.6_W/(1_m*1_K)},
    {11,6,"Au",8,79,"Gold",196.9665,19.3_g/1_cm^3,1337.58_K,3130_K,1.46_Å,0.128_J/(1_g*1_K),"3, 1","[Xe] 4f14 5d10 6s1",2.54,10.2_cm^3/1_mol,9.225,317_W/(1_m*1_K)},
    {12,6,"Hg",8,80,"Mercury",200.59,13.55_g/1_cm^3,234.31_K,629.88_K,1.6_Å,0.140_J/(1_g*1_K),"2, 1","[Xe] 4f14 5d10 6s2",2,14.8_cm^3/1_mol,10.437,8.34_W/(1_m*1_K)},
    {13,6,"Tl",7,81,"Thallium",204.383,11.85_g/1_cm^3,577_K,1746_K,1.71_Å,0.129_J/(1_g*1_K),"3, 1","[Xe] 4f14 5d10 6s2 p1",2.04,17.2_cm^3/1_mol,6.108,46.1_W/(1_m*1_K)},
    {14,6,"Pb",7,82,"Lead",207.2,11.35_g/1_cm^3,600.65_K,2023_K,1.75_Å,0.129_J/(1_g*1_K),"4, 2","[Xe] 4f14 5d10 6s2 p2",2.33,18.3_cm^3/1_mol,7.416,35.3_W/(1_m*1_K)},
    {15,6,"Bi",7,83,"Bismuth",208.9804,9.75_g/1_cm^3,544.59_K,1837_K,1.7_Å,0.122_J/(1_g*1_K),"3, 5","[Xe] 4f14 5d10 6s2 p3",2.02,21.3_cm^3/1_mol,7.289,7.87_W/(1_m*1_K)},
    {16,6,"Po",6,84,"Polonium",209,9.3_g/1_cm^3,527_K,"n/a",1.67_Å,"n/a","4, 2","[Xe] 4f14 5d10 6s2 p4",2,22.7_cm^3/1_mol,8.42,20_W/(1_m*1_K)},
    {17,6,"At",4,85,"Astatine",210,"n/a",575_K,610_K,1.45_Å,"n/a","+/-1, 3, 5, 7","[Xe] 4f14 5d10 6s2 p5",2.2,"n/a","n/a",1.7_W/(1_m*1_K)},
    {18,6,"Rn",5,86,"Radon",222,9.73_g/1_cm^3,202_K,211.4_K,1.34_Å,0.094_J/(1_g*1_K),"n/a","[Xe] 4f14 5d10 6s2 p6",0,50.5_cm^3/1_mol,10.748,0.00364_W/(1_m*1_K)},
    {1,7,"Fr",2,87,"Francium",223,"n/a",300_K,950_K,2.7_Å,"n/a","1","[Rn] 7s1",0.7,"n/a",0,15_W/(1_m*1_K)},
    {2,7,"Ra",3,88,"Radium",226.0254,5_g/1_cm^3,973_K,1413_K,2.33_Å,0.094_J/(1_g*1_K),"2","[Rn] 7s2",0.89,45.2_cm^3/1_mol,5.279,18.6_W/(1_m*1_K)},
    {4,10,"Ac",10,89,"Actinium",227,10.07_g/1_cm^3,"n/a",3470_K,1.88_Å,0.12_J/(1_g*1_K),"3","[Rn] 6d1 7s2",1.1,22.5_cm^3/1_mol,5.17,12_W/(1_m*1_K)},
    {5,10,"Th",10,90,"Thorium",232.0381,11.72_g/1_cm^3,2028_K,5060_K,1.8_Å,0.113_J/(1_g*1_K),"4","[Rn] 6d2 7s2",1.3,19.9_cm^3/1_mol,6.08,54_W/(1_m*1_K)},
    {6,10,"Pa",10,91,"Protactinium",231.0359,15.4_g/1_cm^3,1845_K,4300_K,1.61_Å,"n/a","5, 4","[Rn] 5f2 6d1 7s2",1.5,15_cm^3/1_mol,5.88,47_W/(1_m*1_K)},
    {7,10,"U",10,92,"Uranium",238.029,18.95_g/1_cm^3,1408_K,4407_K,1.38_Å,0.12_J/(1_g*1_K),"6, 5, 4, 3","[Rn] 5f3 6d1 7s2",1.38,12.5_cm^3/1_mol,6.05,27.6_W/(1_m*1_K)},
    {8,10,"Np",10,93,"Neptunium",237.0482,20.2_g/1_cm^3,912_K,4175_K,1.3_Å,"n/a","6, 5, 4, 3","[Rn] 5f4 6d1 7s2",1.36,21.1_cm^3/1_mol,6.19,6.3_W/(1_m*1_K)},
    {9,10,"Pu",10,94,"Plutonium",244,19.84_g/1_cm^3,913_K,3505_K,1.51_Å,0.13_J/(1_g*1_K),"6, 5, 4, 3","[Rn] 5f6 7s2",1.28,12.32_cm^3/1_mol,6.06,6.74_W/(1_m*1_K)},
    {10,10,"Am",10,95,"Americium",243,13.7_g/1_cm^3,1449_K,2880_K,1.84_Å,"n/a","6, 5, 4, 3","[Rn] 5f7 7s2",1.3,20.8_cm^3/1_mol,6,10_W/(1_m*1_K)},
    {11,10,"Cm",10,96,"Curium",247,13.5_g/1_cm^3,1620_K,"n/a","n/a","n/a","3","[Rn] 5f7 6d1 7s2",1.3,18.3_cm^3/1_mol,6.02,10_W/(1_m*1_K)},
    {12,10,"Bk",10,97,"Berkelium",247,"n/a","n/a","n/a","n/a","n/a","4, 3","[Rn] 5f9 7s2",1.3,"n/a",6.23,10_W/(1_m*1_K)},
    {13,10,"Cf",10,98,"Californium",251,"n/a",1170_K,"n/a","n/a","n/a","3","[Rn] 5f10 7s2",1.3,"n/a",6.3,10_W/(1_m*1_K)},
    {14,10,"Es",10,99,"Einsteinium",252,"n/a",1130_K,"n/a","n/a","n/a","n/a","[Rn] 5f11 7s2",1.3,"n/a",6.42,10_W/(1_m*1_K)},
    {15,10,"Fm",10,100,"Fermium",257,"n/a",1800_K,"n/a","n/a","n/a","n/a","[Rn] 5f12 7s2",1.3,"n/a",6.5,10_W/(1_m*1_K)},
    {16,10,"Md",10,101,"Mendelevium",258,"n/a",1100_K,"n/a","n/a","n/a","n/a","[Rn] 5f13 7s2",1.3,"n/a",6.58,10_W/(1_m*1_K)},
    {17,10,"No",10,102,"Nobelium",259,"n/a",1100_K,"n/a","n/a","n/a","n/a","[Rn] 5f14 7s2",1.3,"n/a",6.65,10_W/(1_m*1_K)},
    {18,10,"Lr",10,103,"Lawrencium",262,"n/a",1900_K,"n/a","n/a","n/a","n/a","[Rn] 5f14 6d1 7s2","n/a","n/a","n/a",10_W/(1_m*1_K)},
    {4,7,"Rf",8,104,"Rutherfordium",261,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d2 7s2","n/a","n/a","n/a","n/a"},
    {5,7,"Db",8,105,"Dubnium",262,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d3 7s2","n/a","n/a","n/a","n/a"},
    {6,7,"Sg",8,106,"Seaborgium",263,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d4 7s2","n/a","n/a","n/a","n/a"},
    {7,7,"Bh",8,107,"Bohrium",262,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d5 7s2","n/a","n/a","n/a","n/a"},
    {8,7,"Hs",8,108,"Hassium",265,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d6 7s2","n/a","n/a","n/a","n/a"},
    {9,7,"Mt",8,109,"Meitnerium",266,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d7 7s2","n/a","n/a","n/a","n/a"},
    {10,7,"Uun",8,110,"ununnilium",269,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d8 7s2","n/a","n/a","n/a","n/a"},
    {11,7,"Uuu",8,111,"unununium",272,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d9 7s2","n/a","n/a","n/a","n/a"},
    {12,7,"Uub",8,112,"ununbium",277,"n/a","n/a","n/a","n/a","n/a","n/a","[Rn] 5f14 6d10 7s2","n/a","n/a","n/a","n/a"},
    {13,7,"Uut",7,113,"Ununtrium","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a"},
    {14,7,"Uuq",7,114,"Ununquadium","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a"},
    {15,7,"Uup",7,115,"Ununpentium","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a"},
    {16,7,"Uuh",7,116,"Ununhexium","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a"},
    {17,7,"Uus",4,117,"Ununseptium","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a"},
    {18,7,"Uuo",5,118,"Ununoctium","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a","n/a"}
#END
};

local Periodic.extendedDisplayUsed := CONCAT(4,MAKELIST(X,X,7,18));

local Periodic.extendedDisplayNames := {
    "Group",
    "Atomic Weight",
    "Density",
    "Melting Point",
    "Boiling Point",
    "Atomic Radius",
    "Specific Heat",
    "Oxidation States",
    "Electron Configuration",
    "Electronegativity",
    "Atomic Volume",
    "Ionization Potential",
    "Thermal Conductivity"
};
    
#define CELL_SIZE 17

local Periodic.colorTable := {
    {RGB(226,238,254), RGB(0,96,236)},
    {RGB(215,248,255), RGB(0,117,139)},
    {RGB(255,231,231), RGB(216,0,35)},
    {RGB(254,247,225), RGB(149,87,13)},
    {RGB(255,231,235), RGB(207,29,92)},
    {RGB(213,249,233), RGB(00,44,00)},
    {RGB(243,232,252), RGB(98,51,232)},
    {RGB(223,243,255), RGB(0,51,84)},
    {RGB(255,230,213), RGB(201,50,4)},
    {RGB(231,231,234), RGB(63,55,78)}
};

#define Colors paper[1], ink[2]

local Periodic.chemicalGroups = {"Nonmetal", "Alkali Metal", "Alkaline Earth Metal", "Halogen", "Noble Gas", "Metalliod", "Post-Transition Metal", "Transition Metal", "Lanthanoid", "Actinoid"};

local Periodic.column := 1;
local Periodic.row := 1;

Periodic::RenderCell(c)
begin
    #define Cell column[1], row[2], actinideElementName[3], colorIndex[4], elementAtomicNumber[5], chemicalElementName[6]
    
    local cell;
    dict Cell cell;
    
    cell = c;
    
    def c[1] * CELL_SIZE - 9 x1;
    def c[2] * CELL_SIZE y1;
       
    def eval:(c[1] + 1) * CELL_SIZE - 2 - 9 x2;
    def eval:(c[2] + 1) * CELL_SIZE - 2 y2;
    
    struct Colors auto:colors;
    colors = Periodic.colorTable[cell.colorIndex];
    

    if Periodic.column==cell.column && Periodic.row==cell.row do
        struct Size auto:size;
                
        rect(G1, x1 - 7, y1 - 7, x2 + 7, y2 + 7, colors.ink, colors.paper);
        size := TEXTSIZE(cell.actinideElementName, 3);
        textout(cell.actinideElementName, G1, x1 + 7 - size.width / 2, y1 + 10 - size.height / 2, 3, colors.ink);
        textout(cell.elementAtomicNumber, G1, x1 - 5, y1 - 5, 1, colors.ink);
        
        size := TEXTSIZE(cell.elementAtomicNumber + " - " + cell.chemicalElementName, 4);
        textout(cell.elementAtomicNumber + " - " + cell.chemicalElementName, G1, 160 - size.width / 2, 200, 4, colors.ink);
        return;
    end;
    
    rect(G1, x1, y1, x2, y2, colors.paper, colors.paper);
    textout(cell.actinideElementName, G1, x1 + 2, y1 + 4, 1, colors.ink);
end;



Periodic::DisplayLine(auto:cell, I:index)
begin
    guard index<14 else
        return;
    end;
    
    /// It is necessary to evaluate, as we are referencing an alias 'cell' in the definition.
    def eval:cell[3] actinideElementName;
    def eval:cell[4] colorIndex;
    def eval:cell[5] elementAtomicNumber;
    def eval:cell[6] chemicalElementName;
    
    dict Size auto:size;
    dict Colors colors;
    colors = Periodic.colorTable[colorIndex];
    
    if even(index) == false then
        rect(G1, 0, 31 + 16 * index, 320, 48 + 16 * index, RGB(255,255,255,191));
    end;
    
    if index==1 then
        textout(Periodic.extendedDisplayNames[index], G1, 4, 17 + 16, 2);
        size = TEXTSIZE(Periodic.chemicalGroups[colorIndex], 0);
        textout(Periodic.chemicalGroups[colorIndex], G1, SCREEN_WIDTH - size.width - 4, 15 + 16, 0, colors.ink);
        return;
    end;
    
    
    size = TEXTSIZE(cell[Periodic.extendedDisplayUsed[index]], 0);
    textout(Periodic.extendedDisplayNames[index], G1, 4, 17 + 16 * index, 2);
    textout(cell[Periodic.extendedDisplayUsed[index]], G1, SCREEN_WIDTH - size.width - 4, 15 + 16 * index, 0, Color.Gray);
end;

Periodic::RenderTable()
begin
    DIMGROB_P(G1,320,240);
    rect(G1);
    MAKELIST([Periodic RenderCell:Periodic.table[X]], X, 1, SIZE(Periodic.table));
    
    dict column[1], row[2] cell;
    local I:index;
        
    for index := 1; index <= SIZE(Periodic.table); index += 1 do
        local cell := Periodic.table[index];
        if Periodic.column == cell.column && Periodic.row == cell.row then
            [Periodic RenderCell:cell];
            break;
        end;
    end;
end;

Periodic::ChemicalDataMenu(auto:cell)
begin
    /// It is necessary to evaluate, as we are referencing an alias 'cell' in the definition.
    def eval:cell[3] actinideElementName;
    def eval:cell[4] colorIndex;
    def eval:cell[5] elementAtomicNumber;
    def eval:cell[6] chemicalElementName;
    
    local I:index := elementAtomicNumber;

    do
        struct Colors colors;
        colors = Periodic.colorTable[colorIndex];
    
        defGROB(G1, SCREEN_WIDTH, SCREEN_HEIGHT, colors.paper);
        rect(G1,0,0,SCREEN_WIDTH, 30, Color.Black);
        textout(elementAtomicNumber + " - " + chemicalElementName + " (" + actinideElementName + ")", G1, 4, 2, 5, Color.White);
        MAKELIST([Periodic DisplayLineWithCell:cell index:X], X, 1, SIZE(Periodic.extendedDisplayUsed));
        
        cartesian::blit(G1);

        local auto:input := WAIT(0);

        switch input
            case KeyCode.Esc do
                return;
            end;

            case KeyCode.Left do
                index-=1;
                if index==0 then index:=118; end;
                cell := Periodic.table[index];
            end;

            case KeyCode.Right do
                index+=1;
                if index==119 then index:=1; end;
                cell := Periodic.table[index];
            end;
        end;
    end;
end;

Periodic::DoesCellExist(c, r)
begin
    dict column[1], row[2] cell;
    
    local I:index;
        
    for index := 1; index <= SIZE(Periodic.table); index += 1 do
        local cell := Periodic.table[index];
        if c == cell.column && r == cell.row do
            return true;
        end;
    end;
end;


Periodic::OnTouch(p:input)
begin
    struct Event auto:event;
    event = input;
        
    switch event.type
        case EventType.MouseClick do
            if event.x >= 53 then return;
            if event.y <= 220 then return;
            io::message::box("HP Prime Periodic Table.");
        end;
        
        case EventType.MouseDown do
            if event.y > 200 then return;
            local column = floor((event.x + 9) / CELL_SIZE);
            local row = floor(event.y / CELL_SIZE);
            
            if [Periodic DoesCellExistAtCoordinates:column ofRow: row] == true do
                Periodic.column = column;
                Periodic.row = row;
                return;
            end;
        end;
    end;
end;

Periodic::OnKeyPress(auto:keyPressed)
begin
    
    dict column[1], row[2] cell;
                    
    switch keyPressed
        case KeyCode.Enter do
            local I:index;
            
            for index = 1; index <= SIZE(Periodic.table); index += 1 do
                cell = Periodic.table[index];
                if Periodic.column==cell.column && Periodic.row==cell.row then break; end;
            end;
            [Periodic ChemicalDataMenu:cell];
        end;
        
        case KeyCode.Left do
            repeat
                Periodic.column := (Periodic.column - 2) % 18 + 1;
            until Periodic::DoesCellExist(Periodic.column, Periodic.row) == true;
        end;
        
        case KeyCode.Right do
            repeat
                Periodic.column := Periodic.column % 18 + 1;
            until Periodic::DoesCellExist(Periodic.column, Periodic.row) == true;
        end;
        
        case KeyCode.Up do
            repeat
                Periodic.row := (Periodic.row - 2) % 10 + 1;
            until Periodic::DoesCellExist(Periodic.column, Periodic.row) == true;
        end;
        
        case KeyCode.Down do
            repeat
                Periodic.row := Periodic.row % 10 + 1;
            until Periodic::DoesCellExist(Periodic.column, Periodic.row) == true;
        end;
    end;
end;

Periodic::Update()
begin
    [Periodic RenderTable];
    BLIT(G1);
    drawMenu("About");
end;


export PERIODIC()
begin
    do
        Periodic::Update();
        local auto:input = WAIT(-1);
        
        if TYPE(input) == ObjectType.List then
            Periodic::OnTouch(input);
        else
            if input == KeyCode.Esc then
                Periodic.column := 0; Periodic.row := 0;
                return;
            end;
            Periodic::OnKeyPress(input);
        end;
    end;
end;

