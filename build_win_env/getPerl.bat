SET TEMP_DIR=C:\MS35TEMP
IF NOT EXIST %TEMP_DIR% MKDIR %TEMP_DIR%
CD %TEMP_DIR%
curl -O -L http://www.mect.it/archive/qwt-6.1-multiaxes.tar.bz2