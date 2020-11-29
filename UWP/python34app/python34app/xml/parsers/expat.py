"""Interface to the Expat non-validating XML parser."""
import sys

from importlib.machinery import ExtensionFileLoader

def unbulk_dyn_load_package_name(module_name, package_name, extension_name):
    foo = ExtensionFileLoader(package_name, extension_name).load_module()
    sys.modules[module_name] = foo
    return foo    

pyexpat = unbulk_dyn_load_package_name("pyexpat", "pyexpat", "pyexpat.pyd")

from pyexpat import *

# provide pyexpat submodules as xml.parsers.expat submodules
sys.modules['xml.parsers.expat.model'] = model
sys.modules['xml.parsers.expat.errors'] = errors
