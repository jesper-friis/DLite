from ontopy import get_ontology

# Note install emmopython from github, not pypi. 

mapping_onto = get_ontology('mapping.ttl').load(EMMObased=False)

chemistry_onto = get_ontology('chemistry.ttl').load()

dlite_onto = get_ontology('https://raw.githubusercontent.com/'
                          'emmo-repo/datamodel-ontology/master'
                          '/dlitemodel.ttl').load(EMMObased=False)



substance_model = dlite_onto.Metadata('http://onto-ns.com/meta/0.1/Substance')
substance_model.mapsTo = chemistry_onto.MoleculeModel


substance_energy = dlite_onto.Metadata('http://onto-ns.com/meta/0.1/Substance#energy')
substance_energy.mapsTo = chemistry_onto.Energy






