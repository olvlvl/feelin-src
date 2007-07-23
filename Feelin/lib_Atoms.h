#ifndef LIB_ATOM_H
#define LIB_ATOM_H

struct in_LocalAtom
{
    struct in_LocalAtom            *next;
    FAtom                          *atom;
    uint32                          refs;
};

struct in_LocalAtomPool
{
    APTR                            pool;
    struct in_LocalAtom            *local_atoms;
};

#define F_LATOM_PUBLICIZE(latom)                            ((latom)->atom)
#define F_LATOM_PRIVATIZE(latom)                            (struct in_LocalAtom *) ((uint32)(latom) - sizeof (struct in_LocalAtom *))

struct in_Atom
{
    struct in_Atom                 *next;
    FAtom                           public;
    uint32                          refs;
};

#define F_ATOM_PUBLICIZE(prv)                   (FAtom *)((uint32)(prv) + sizeof (struct in_Atom *))
#define F_ATOM_PRIVATIZE(pub)                   (struct in_Atom *)((uint32)(pub) - sizeof (struct in_Atom *))

#endif

