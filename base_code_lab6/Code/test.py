def minimum(liste):
    assert len(liste) > 0 # précondition
    valeur_min = liste[0] #premiere valeur de la liste
    for valeur in liste:
        if valeur < valeur_min:
            valeur_min = valeur
    return valeur_min



print(minimum(['toto', 102, 72]))