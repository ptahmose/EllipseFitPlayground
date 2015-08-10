SET EXE=..\Debug\EllipseUtils.exe

FOR /L %%I IN (0,1,308) DO (
%EXE% --command leastsquarefit --points .\ArcTest_%%I.txt --svg .\output\fittedellipse_%%I.svg  
)

