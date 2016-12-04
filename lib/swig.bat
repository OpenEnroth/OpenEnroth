rem echo 1 %1
rem echo 2 %2
rem echo 3 %3
rem echo 4 %4
rem echo 5 %5
rem echo 6 %6

set WOMM_SWIG_SOLUTION_DIR=%CD%

rem echo xcopy %4 "../../lib/swig/swigwin-2.0.11" /y
xcopy %4 "../../lib/swig/swigwin-2.0.11" /y

cd ../../lib/swig/swigwin-2.0.11
swig -c++ -lua %3

rem echo xcopy "%6" %5"%2" /y
xcopy "%6" %5"%2" /y

rem echo chdir /d %WOMM_SWIG_SOLUTION_DIR%
chdir /d %WOMM_SWIG_SOLUTION_DIR%