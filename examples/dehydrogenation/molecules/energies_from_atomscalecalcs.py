import os

import dlite
import ase.io
from ase.calculators.emt import EMT

def readMolecule(filename, Molecule):
    atoms = ase.io.read(filename)  # ASE Atoms object
    atoms.calc = EMT()
    basename = os.path.splitext(filename)[0]
    inst = Molecule(dims=[len(atoms), 3], id=basename)  # DLite instance
    inst.symbols = atoms.get_chemical_symbols()
    inst.masses = atoms.get_masses()
    inst.positions = atoms.positions
    inst.energy = atoms.get_potential_energy()
    return inst

Molecule = dlite.Instance('json:Molecule.json')  # DLite Metadata

coll = dlite.Collection('molecules')
for molname in ['c2h6', 'c2h4', 'h2']:
    mol = readMolecule(molname+'.xyz', Molecule)
    coll.add(label=molname, inst=mol)

coll.save('json', 'atomscaledata.json', 'mode=w')
