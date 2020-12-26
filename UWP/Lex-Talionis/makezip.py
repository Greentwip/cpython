#!python3.4
import compileall, os, zipfile, imp, sys
force = False
#force = True

print(sys.version)

#imp_suffix = "."+imp.get_tag()+".pyc"

out = zipfile.ZipFile('Lex-Talionis.zip', 'w')#, compression=zipfile.ZIP_DEFLATED)
def compress(prefix, arcname=None):
  for dir, subdirs, files in os.walk(prefix):
    if dir.startswith('.'):
      continue
    for d in subdirs[:]:
      if d in ('test', 'tests') or d.startswith('plat-'):
        subdirs.remove(d)

    ddir = None
    if arcname is None:
        ddir = dir[len(prefix):]
    else:
        ddir = os.path.basename(arcname)# dir[len(arcname):]
        if "src_py" in os.path.basename(arcname) + "/" + os.path.basename(dir):
    		    ddir = os.path.basename(arcname)
        else: 
            ddir = os.path.basename(arcname) + "/" + os.path.basename(dir)
    #print(os.path.basename(arcname) + "/" + os.path.basename(dir))




    for f in files:
      if f.endswith('.py'):
          out.write(os.path.join(dir, f), ddir+"/"+f)
      else:
        continue

prefix = os.path.abspath('./../../Lib/')

compress(prefix)

prefix = os.path.abspath('./../Libraries/Pygame/Source/src_py/')
arcname = os.path.abspath('./../Libraries/Pygame/Source/pygame/')

compress(prefix, arcname)

out.close()

