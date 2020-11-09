import metroui, sys, codeop

class PseudoFile:
    def __init__(self, readline=None, write=None):
        self.readline=readline
        self.write=write

    def writelines(self, lines):
        for line in lines:
            self.write(line)

    def flush(self):
        pass

    def isatty(self):
        return True

    def read(self, *args):
        raise NotImplementedError

def eval(code):
    try:
        exec(code, globals())
    except SystemExit:
        metroui.exit()
        return

compile = codeop.CommandCompiler()

sys.stdout = PseudoFile(write=metroui.add_to_stdout)
sys.stderr = PseudoFile(write=metroui.add_to_stderr)
sys.stdin = PseudoFile(readline=metroui.readline)
#sys.stdout = open(os.devnull, 'w')
#sys.stderr = open(os.devnull, 'w')

print("Python %s on %s" % (sys.version, sys.platform))
print('Type "help", "copyright", "credits" or "license" for more information.')

