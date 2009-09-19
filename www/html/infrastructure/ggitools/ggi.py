import sys
import os

from ggitools import tools

# Configuration

htmldest   = os.getenv('GGIHTML',      None)
repository = os.getenv('GGICVS',       None)
packageDir = os.getenv('GGIPACKAGES',  None)

useTree = os.getenv('GGIUSETREE', 'yes') != 'no'

if None in (repository, htmldest, packageDir):
    print "Please set GGIHTML, GGICVS and GGIPACKAGES envar"
    sys.exit(1)

# Global

families     = []
branches     = []
releases     = []
releaseNames = []
packages     = []

def _parse():
    _branches, _families, _packages = tools.listBranchesAndFamilies(
        os.path.join(repository, 'html/infrastructure/packages'),
        packageDir,
        repository,
        useTree)
    _releases, _releaseNames = tools.listReleases(
        os.path.join(repository, 'html/infrastructure'),
        _branches
        )
    families[:] = _families
    branches[:] = _branches
    releases[:] = _releases
    releaseNames[:] = _releaseNames
    packages[:] = _packages
_parse()


# Sorting

def byFamily(a,b):
    return cmp(families.index(a.family), families.index(b.family))
def byPackage(a,b):
    return cmp(packages.index(a.pkgname), packages.index(b.pkgname))
def byRelease(a,b):
    if a.release == 'current': return -1
    if b.release == 'current': return 1
    return cmp(a.release, b.release)
def byBranch(a,b):
    if a.branch == 'current': return  1
    if b.branch == 'current': return -1
    return cmp(a.branch, b.branch)

# Actions

def do_cvsup():
    """Create a script that will update the cvs copy"""
    tools.sortBy(branches, byFamily, byPackage, byBranch)
    f = file(os.path.join(repository,'html/tmp/cvsup.sh'), 'w')
    f.write('. %s\n'%os.path.join(repository,'html/infrastructure/helpers.sh'))
    for b in branches:
        if not b.real == b.pkgname: continue
        f.write('cvsUpdatePackage %s %s %s\n'%(b.directory,
                                               b.path,
                                               b.tag and '"-r %s"'%b.tag))

def do_releasesIndex():
    """Creates releases index files from release list."""
    releases = {}
    print ">> generating release list"
    for i in [ j.strip() for j in file(os.path.join(repository,'html/tmp/pages'))
               if j.startswith('packages/') and '-' in j ]:
        pkg, version = os.path.basename(i)[:-4].split('-')
        if not pkg in releases:
            releases[pkg] = {}
        version = tuple([ int(j) for j in version.split('.') ])
        releases[pkg][version] = [
            j.strip().split()[-2:]
            for j in file(os.path.join(repository,'html/%s'%i))
            if j.startswith(".. hotfix::") ]
    o = file(os.path.join(repository, 'html/tmp/releases.new'),'w')
    o.write(tools.title('Releases'))
    for pkg in [ i for i in branches if i.tag == '' ]:
        if pkg.pkgname in releases:
            t = pkg.name + " - " + pkg.descr
            o.write("\n\n" + t + "\n" + '-'*len(t)+"\n\n")
            k = releases[pkg.pkgname].keys()
            k.sort()
            k.reverse()
            for j in k:
                n = len(releases[pkg.pkgname][j])
                fix = n and " - %i fix%s"%(n, n != 1 and 'es' or '') or ''
                ver = '.'.join([ str(i) for i in j])
                o.write("- `%s %s <%s.html>`__%s\n"%(
                    pkg.pkgname,ver,pkg.pkgname+'-'+ver,fix))
    o.close()

def do_latestNews(recent = 5):
    print ">> generating lastest news"
    source = os.path.join(repository, 'html/news.txt')
    target = os.path.join(repository, 'html/tmp/latestNews')
    f =  file(source)
    for i in xrange(3): f.readline()
    r = [ ' ', ' ']
    for l in f:
        if l.strip() and l.strip() == '-'*(len(r[-1].strip())):
            if recent == 0: break
            recent -= 1
        r.append(l)
    file(target,'w').write(''.join(r[:-2]))


def do_site():
    import html
    context = tools.Context(htmldest,
                            [ i for i in branches if i.branch=='current'])
    print ">> publishing site in", htmldest
    html.publishSite(context)
    html.publishHTMLDocumentation(context)
    
    tools.sortBy(releases, byRelease)
    r = [ i for i in releases if i.release != 'current']
    while r:
        t = r
        r, c = [],[]
        for i in t:
            if i.release == t[0].release: c.append(i)
            else: r.append(i)
        context = tools.Context(htmldest, c)
        html.publishHTMLDocumentation(context)


def do_manpages():
    import man
    context = tools.Context(htmldest,
                            [ i for i in branches if i.branch=='current'])
    man.updateManpages(context)
    tools.sortBy(releases, byRelease)
    r = [ i for i in releases if i.release != 'current']
    while r:
        t = r
        r, c = [],[]
        for i in t:
            if i.release == t[0].release: c.append(i)
            else: r.append(i)
        context = tools.Context(htmldest, c)
        man.updateManpages(context)


def do_list():
    for b in branches:
        print b.pkgname, b.branch, b.directory

if __name__ == '__main__':
    def unknown(name):
        def _():
            print "%s: don't known how to do '%s'"%(sys.argv[0], name)
        return _
    for i in sys.argv[1:]:
        globals().get('do_%s'%i, unknown(i))()
