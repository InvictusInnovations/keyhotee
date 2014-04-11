import shutil
import os
import zipfile

version = "0.6.0"

def zipdir(path, zip):
    for root, dirs, files in os.walk(path):
        for file in files:
            zip.write(os.path.join(root, file))


if __name__ == '__main__':
    #copy files listed in install_lst.txt to temporary bin dir
    binDir = 'keyhotee_' + version
    if os.path.exists(binDir):
        shutil.rmtree(binDir)
    #TODO: remove dependency on msvcr100 dll (from ICU dlls)		
    os.mkdir(binDir)
    for line in open("keyhotee_install_32.txt",'r'):
        line=line.strip()
        if line:
            print("copy " + line)
            shutil.copy(line,binDir)
    #platformDir = binDir + '/platforms'
    #os.mkdir(platformDir)
    #print('platformDir=',platformDir)
    #shutil.copy('c:/gh/bin/platforms/qminimal.dll',platformDir)
    #shutil.copy('c:/gh/bin/platforms/qoffscreen.dll',platformDir)
    #shutil.copy('c:/gh/bin/platforms/qwindows.dll',platformDir)
    
    #zip up files in bin dir to miner.zip
    zipfileName = binDir + '.zip'
    zip = zipfile.ZipFile(zipfileName,'w')
    zipdir(binDir, zip)
    zip.close()
    #copy pdb 
    pdbDir = 'pdbs' + version
	if os.path.exists(pdbDir)
		shutil.rmtree(pdbDir)
	os.mkdir(pdbDir)
    shutil.copy('c:/gh/vs11/release/',pdbDir)
    #zipfileName = pdbDir + '.zip'
    #zip = zipfile.ZipFile(zipfileName,'w')
	#zipdir(pdbDir,zip)
	#zip.close()
