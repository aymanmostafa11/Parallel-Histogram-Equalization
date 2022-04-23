cd Debug
set /p proc= "Enter Number of processors : "
mpiexec -np %proc% "Parallel_Histogram_Equalization.exe"
@pause