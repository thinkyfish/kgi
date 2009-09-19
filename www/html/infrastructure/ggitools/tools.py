"""
Various utilities for ReST files and ggi content.
"""

import os
import itertools

    
class D:
    def __init__(self, **kw): self.__dict__.update(kw)
    def copy(self):
        return D(**dict([ (i, getattr(self, i)) for i in self.__dict__.keys()
                          if i[0] != '_' ]))

def select(list, **kw):
    def _(a):
        for k,v in kw.iteritems():
            if getattr(a, k, None) != v: return False
        return True
    return itertools.ifilter(_, list)

def selectOne(list, **kw):
    return select(list, **kw).next()

def lines(filename):
    "Simple file parser, ingoring empty lines and '#' comments "
    for i in file(filename):
        a = i.strip()
        if a and a[0] != '#': yield a.split()

def sortBy(list, *cmps):
    def _(a,b):
        for c in cmps:
            r = c(a,b)
            if r: return r
        return 0
    list.sort(_)

class Context:
    def __init__(self, destination, packages):
        self.destination = destination # HTML target dir
        self.terms       = {}   # For the glossary
        self.package     = None # Currently active package, if any
        self.packages    = []   # Packages in the package set (e.g. release)
        
        for p in packages:
            self.packages.append(p.copy())
            doc = os.path.join(p.directory,'doc/%s.txt'%p.pkgname)
            print ">> reading manpages from", doc
            self.packages[-1].manpages = scanManpages(doc,recursive=True)
            self.packages[-1].docfile  = doc
            
    def findPage(self, page, volume = None):
        r = []
        def _inpkg(pkg):
            for m in pkg.manpages:
                if volume and m.volume != volume:
                    continue
                if page in m.pages:
                    r.append(D(type='man',
                               volume=m.volume,
                               entry=m.pages[0],
                               pkgname=pkg.pkgname,
                               branch=pkg.branch))
        if self.package: _inpkg(self.package)
        for p in self.packages:
            if not p == self.package: _inpkg(p)
        return r
    
    def findTerm(self, term):
        r = self.findPage(term)
        if term in self.terms:
            r.append(('term', self.terms[term]))
        return r

def scanManpages(filename, recursive = False):
    """
    Return a list of manpages defined in a file as
    a list of D(pages,title,volume,filemane)
    """
    title = ''
    previous = ''
    res   = []
    
    if not os.path.isfile(filename):
        print '*> %s: no such file'%filename
        return []
    for i in file(filename):
        i = i.strip()
        if recursive and i.startswith('.. include::'):
            res.extend(scanManpages(os.path.join(os.path.dirname(filename),
                                                 i.split()[2]), recursive))
        elif i.startswith('.. manpage::'):
            t = i[len('.. manpage::'):].split()
            res.append(D(filename=filename,pages=t[1:],volume=t[0],title=title))
        elif i and i.count(i[0]) == len(i):
            title = previous
        else:
            previous = i
    return res


def getTitle(filename):
    """Get the title of the given page
    
    Heuristic: the title is the first line containig more than
    one character.
    """
    for i in file(filename):
        i = i.strip()
        if i and i.count(i[0]) != len(i):
            return i
        
def title(t,char='='):
    return '\n%s\n%s\n%s\n\n'%(char*len(t),t,char*len(t))


def includedFiles(filename):
    """List all file included from a given ReST File.
    
    Return a list of file names.
    Deals properly with (forbidden) cyclic inclusion.
    """
    if not os.path.isfile(filename):
        print '*> %s: no such file'%filename
        return []
    def _(done, left):
        todo = list(left)
        for filename in left:
            for i in file(filename):
                if i.startswith('.. include::'):
                    n = os.path.join(os.path.dirname(filename),
                                     i.strip().split()[2])
                    if n in todo or n in done: continue
                    todo.append(n)
            todo.remove(filename)
            done.append(filename)
        if todo: return _(done, todo)
        return done
    return _([], [filename])

timestampFiles = os.getenv('GGITIMESTAMPFILES')
timestampFiles = timestampFiles and timestampFiles.split(':') or []

def needUpdate(destination, dependencies):
    """Check if the given file needs update wrt to its dependencies.
    
    If a dependency contains as a '.. ggi::' directive, it always has
    to be updated
    """
    if os.getenv("FORCEUPDATE"):
        return True
    if isinstance(dependencies, str): dependencies = [ dependencies ]
    if not os.path.isfile(destination):
        #print '*  "%s" does not exist'%destination
        return True
    dt = os.path.getmtime(destination)
    dependencies += timestampFiles
    for i in dependencies:
        if not os.path.isfile(i):
            print '*> %s: not such file'%i
            continue
        if os.path.getmtime(i) > dt:
            #print '*  "%s" is younger than "%s"'%(i, destination)
            return True
        for f in file(i):
            if f.startswith('.. ggi::'): return True
    return False


def listBranchesAndFamilies(source, pkgDir, repDir, useTree=True):
    families = []
    packages = []
    res = []
    tmp = []
    family = '???'
    #print ">> parsing branches from", source
    for a in lines(source):
        if a[0] == '-':
            family = ' '.join(a[1:])
            if not family in families: families.append(family)
            continue
        if not a[0] in packages:
            packages.append(a[0])
        if not ':' in a[1]:
            res.append(D(pkgname=a[0],branch='current',path=a[1], tag='',
                         name=a[2], descr=' '.join(a[3:]), family=family,
                         real = a[0]))
        else:
            # branch
            loc, tag = a[1].split(':')
            r = selectOne(res, pkgname=a[0], branch='current').copy()
            r.tag    = tag
            r.loc    = loc
            r.branch = a[2]
            res.append(r)
    for a in res:
        if not '@' in a.path: continue
        a.real = a.path[1:]
        d = selectOne(res, pkgname=a.path[1:], tag=a.tag)
        a.path = d.path
    def _sort(x,y):
        return cmp(packages.index(x.pkgname), packages.index(y.pkgname))
    res.sort(_sort)
    for p in res:
        if p.branch == 'current' and useTree:
            p.directory = os.path.join(repDir, p.path)
        else:
            p.directory = os.path.join(pkgDir, p.real, p.branch)
    return res, families, packages


def listReleases(sourceDir, branches):
    res = []
    releaseNames = [ 'current' ]
    for b in branches:
        if b.branch == 'current':
            r = b.copy()
            r.release = 'current'
            res.append(r)

    
    for l in lines(os.path.join(sourceDir, 'releases')):
        if l[0] == '-':
            rel = l[2].replace('.','')
            releaseNames.append(rel)
            continue
        pkgname, version = l
        r = selectOne(branches, pkgname=pkgname, branch=version).copy()
        r.release = rel
        res.append(r)
    def _(a,b):
        if a == 'current': return -1
        if b == 'current': return 1
        return cmp(b,a)
    releaseNames.sort(_)
    return res, releaseNames
