#!python3.4
import compileall, os, zipfile, imp, sys
force = False
#force = True

print(sys.version)

#imp_suffix = "."+imp.get_tag()+".pyc"

out = zipfile.ZipFile('Lex-Talionis.zip', 'w')#, compression=zipfile.ZIP_DEFLATED)
def compress(prefix):
  for dir, subdirs, files in os.walk(prefix):
    if dir.startswith('.'):
      continue
    for d in subdirs[:]:
      if d in ('test', 'tests') or d.startswith('plat-'):
        subdirs.remove(d)
    ddir = dir[len(prefix):]
    for f in files:
      if f.endswith('.py'):
          out.write(os.path.join(dir, f), ddir+"/"+f)
      else:
  	    continue

prefix = os.path.abspath('Assets/Lex-Talionis/')

#compress(prefix)

#prefix = os.path.abspath('./../python34app/python34app/')
prefix = os.path.abspath('./../../Lib/')

compress(prefix)

out.close()

