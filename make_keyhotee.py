import shutil
import os
import zipfile

version = "0.5.0"

def zipdir(path, zip):
    for root, dirs, files in os.walk(path):
        for file in files:
            zip.write(os.path.join(root, file))


if __name__ == '__main__':
    #copy files listed in install_lst.txt to temporary bin dir
    binDir = 'keyhotee_' + version
    if os.path.exists(binDir):
        shutil.rmtree(binDir)
    os.mkdir(binDir)
    for line in open("keyhotee_install_32.txt",'r'):
        line=line.strip()
        if line:
            print(line)
            shutil.copy(line,binDir)
    
    #zip up files in bin dir to miner.zip
	zipfileName = binDir + '.zip'
    zip = zipfile.ZipFile(zipfileName,'w')
    zipdir(binDir, zip)
    zip.close()
        