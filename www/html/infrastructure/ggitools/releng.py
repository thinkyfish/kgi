import os
import time
import datetime

import tags as T
import parser
import html

_bg = { 'successful': '#cfc',
        'failed':     '#f88',
        'building':   '#88f' }

_fg = { 'successful': 'green',
        'failed':     'red',
        'building':   'blue' }

def sortByDate(a, b):
    return cmp(a['date'], b['date'])

def sortByPackageAndDate(a, b):
    r = cmp((a['pkgname'],a['version']),
            (b['pkgname'],b['version']))
    if r: return r
    return -cmp(a['date'], b['date'])

def sortByVersionAndArch(a, b):
    r = cmp((a['version'],a['arch']), (b['version'],b['arch']))
    if r: return r
    return -cmp(a['date'], b['date'])

def statsFor(builds, **kw):
    res = { 'successful': 0,
            'failed':    0,
            'building':  0 }
    def _count(i):
        for k,v in kw.iteritems():
            if i[k] != v: return
        res[i['status']] += 1
    for i in builds: _count(i)
    res['total'] = sum(res.values())
    return res

def latestPerArch(builds):
    keep = {}
    for b in builds:
        k = (b['pkgname'],b['version'], b['arch'])
        if k not in keep: keep[k] = []
        keep[k].append(b)
        
    res = []
    for k, v in keep.iteritems():
        v.sort(sortByDate)
        if v:
            res.append(v[-1])
    return res


def buildMatrix(builds, archs):
    builds = latestPerArch(builds)
    builds.sort(sortByPackageAndDate)
    last = [None]
    def _header():
        style  = 'border: 1px solid black;'
        def __(s):
            for n, i in enumerate(s[1:]):
                if n != 0: yield T.br
                yield i
        yield T.tr[
            T.td(colspan='3', rowspan='2', style='text-align: center;')[T.strong['Build matrix']],
            [ [ T.td(style=style+'text-align:center;')[
            T.a(href=i[0]+'.html')[__(i)]] for i in archs ] ]
            ]
        def __(a):
            stats = statsFor(builds, arch=a[0])
            return T.td(style=style+'text-align:center; width:10%;')[
                stats['successful'], '/', stats['total'] ]
        yield T.tr [ [ __(i) for i in archs ] ]
        
    def _archfor(b,a):
        all = [ i for i in b if i['arch'] == a[0] ]
        all.sort(sortByDate)
        return all
    
    def _row(pkgname, version):
        style  = 'border: 0px solid black;'
        style += 'border-left-width: 1px; border-right-width: 1px;'
        if last[0][0] != pkgname:
            style=style+'border-top-width: 1px;'
            yield T.td(style=style)[T.a(href=pkgname+'.html')[pkgname]]
            yield T.td(style=style)[version]
        else:
            yield T.td(style=style)['']
            yield T.td(style=style)[version]
            
        stats = statsFor(builds, pkgname=pkgname, version=version)
        yield T.td(style=style+'padding-left: .5em; padding-right:.5em;')[
            stats['successful'], '/', stats['total']
            ]
        b = [ i for i in builds
              if (i['pkgname'],i['version'])==(pkgname,version) ]
        for a in archs:
            c = _archfor(b, a)
            if not c:
                yield T.td(_style=style+'background: white;')
            else:
                r = c[-1]
                yield T.td(style=style+'text-align: center; background: %s;'%_bg[r['status']])[
                    T.a(href=r['log'],style='color: black;')[r['date']]]
    def _():
        yield _header()
        all = []
        for b in builds:
            if not (b['pkgname'], b['version']) in all:
                all.append((b['pkgname'], b['version']))
        last[0] = (None, None)
        for a in all:
            yield T.tr[_row(*a)]
            last[0] = a
    return T.table(style='border: 1px solid black; font-size: small; width: 100%; border-collapse: collapse')[_()]


class PerArch(html.Page):
    def __init__(self, arch, builds, ctx):
        html.Page.__init__(self, ctx)
        self.arch = arch
        self.builds  = builds
        
    def getTitle(self):
        return ' '.join(self.arch)
    
    def getContent(self):
        yield T.h1[ 'Build history for ', ' '.join(self.arch) ]
        yield self._getMatrix()

    def _getMatrix(self):
        self.builds.sort(sortByPackageAndDate)
        
        pkgnames = []
        for b in self.builds:
            if b['pkgname'] not in pkgnames: pkgnames.append(b['pkgname'])

        def _(builds):
            for b in builds:
                yield T.tr[
                    T.td[ self.last != b['version'] and b['version'] or '' ],
                    T.td[ T.a(href=b['log'])[b['date']] ],
                    T.td(style='color: %s'%_fg[b['status']])[
                    b['status'].upper()]
                    ]
                self.last = b['version']
                
        for pkgname in pkgnames:
            yield T.h2[ pkgname ]
            blds = [ b for b in self.builds if b['pkgname'] == pkgname ]
            self.last = None
            yield T.table[_(blds)]


class PerPackage(html.Page):
    def __init__(self, pkgname, builds, ctx):
        html.Page.__init__(self, ctx)
        self.builds = builds
        self.pkgname = pkgname
        
    def getTitle(self):
        return self.pkgname
    
    def getContent(self):
        yield T.h1['Build history for ', self.pkgname ]
        yield self._getMatrix()
        
    def _getMatrix(self):
        self.builds.sort(sortByVersionAndArch)
        
        branches = []
        for b in self.builds:
            if b['version'] not in branches: branches.append(b['version'])

        def _(builds):
            for b in builds:
                yield T.tr[
                    T.td[ self.last != b['arch'] and b['arch'] or '' ],
                    T.td[ T.a(href=b['log'])[b['date']] ],
                    T.td(style='color: %s'%_fg[b['status']])[
                    b['status'].upper()]
                    ]
                self.last = b['arch']
                
        for branch in branches:
            yield T.h2[ self.pkgname,'-', branch ]
            blds = [ b for b in self.builds if b['version'] == branch ]
            self.last = None
            yield T.table[_(blds)]


def buildsAndArchs(source):
    builds = []
    f = file(source)
    l = f.readline().strip()
    while(l):
        path, status = l.split()
        _, pkgname,version, date, arch, filename = path.split('/')
        log = '/'.join([ 'logs', pkgname,version, date, arch, 'build.log.gz' ])
        builds.append({'pkgname':pkgname,
                       'version':version,
                       'arch':arch,
                       'date':date,
                       'status':status.lower(),
                       'log':os.path.join(log)})
        l = f.readline().strip()
    # It is important to sort the builds not
    builds.sort(sortByPackageAndDate)
    _archs = []
    for b in builds:
        if not b['arch'] in _archs:
            _archs.append(b['arch'])
            
    archs = []
    for i in _archs:
        archs.append([i]+i.split('_'))
    return builds, archs


def publishMatrix(source, destination, ctx):
    if not os.path.isfile(source):
        print 'Warning: "%s": no such file'%source
        return
    if not os.path.isdir(destination):
        os.makedirs(destination)
    
    builds, archs = buildsAndArchs(source)
    
    ctx.navigation = parser.Navigation(up='index.html')
    ctx.navigation.timestamp = datetime.datetime.fromtimestamp(
        os.path.getmtime(source))
    ctx.navigation.submenu = [('Releng', 'index.html')]
    ctx.depth = 1
    
    for a in archs:
        dest = os.path.join(destination,a[0]+'.html')
        page = PerArch(a[1:], [ b for b in builds if b['arch']==a[0] ], ctx)
        print 'P ', dest
        page.writeToFile(dest)
        
    pkgnames = []
    for b in builds:
        if not b['pkgname'] in pkgnames:
            pkgnames.append(b['pkgname'])
    for p in pkgnames:
        dest = os.path.join(destination,p+'.html')
        page = PerPackage(p, [ b for b in builds if b['pkgname']==p ], ctx)
        print 'P ', dest
        page.writeToFile(dest)
