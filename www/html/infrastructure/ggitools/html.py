import os
import datetime
import shutil
import cgi

from ggitools import tags as T
from ggitools import ggi
from ggitools import parser
from ggitools import tools

debug_references  = False


def get_children(node):
    if hasattr(node, 'get_children'):
        return node.get_children()
    return node.children


class Page(object):
    
    def __init__(self, context):
        self.context = context

    def writeToFile(self, dest):
        f = file(dest, 'wb')
        def write(thing):
            if isinstance(thing, unicode):
                f.write(cgi.escape(thing.encode('utf-8')))
            elif isinstance(thing, str):
                f.write(cgi.escape(thing))
            elif isinstance(thing, (int, float, complex)):
                f.write(repr(thing))
            elif isinstance(thing, T.Tag):
                attrs = [ '%s="%s"' %( k, cgi.escape(str(v)))
                          for k,v in thing.attributes.iteritems() ]
                f.write("<%s %s" % (thing.name, ' '.join(attrs)))
                if not thing.children:
                    f.write(" />")
                else:
                    f.write(">")
                    for c in thing.children:
                        write(c)
                    f.write("</%s>" % thing.name)
            elif isinstance(thing, T.raw):
                f.write(thing.content)
            else:
                for c in thing:
                    write(c)
        write(self.render())
        f.close()

    def render(self):
        yield T.raw("""<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">""")
        yield T.html(xmlns="http://www.w3.org/1999/xhtml")[
            T.head[
              T.title[ self.getTitle() ],
              T.link(rel="stylesheet",
                     type="text/css",
                     href=self.getStylesheet()),
              T.meta(content="text/html; charset=UTF-8",
                     **{'http-equiv':"Content-type"})
            ],
            T.body[
              T.div(id="page")[
                T.div(id="header")[
                  T.div(class_="menu-global")[ self.getGlobalMenu() ],
                  T.div(class_="menu-local")[ self.getLocalMenu() ]
                ],
                T.div(id="content")[ self.getContent() ],
                T.div(id="footer")[
                  T.div(class_="links")[
                    T.a(href="http://ibiblio.org/")[
                      T.img(width="88",
                            src="http://www.ibiblio.org/images/ibiblio_logo.gif",
                            height="31", alt="iBiblio")
                    ],
                    T.a(href="http://sourceforge.net/projects/kgi")[
                      T.img(width="88",
                            src="http://sourceforge.net/sflogo.php?group_id=3788",
                            height="31", alt="Sourceforge")
                    ]                    
                  ]
                ],
              ]
            ]
        ]
    
        
    def getGlobalMenu(self):
        for name, href in self.context.navigation.menu:
            yield T.a(href='../'*self.context.depth + href)[name]
            yield ' '
    
    def getLocalMenu(self):
        n = self.context.navigation
        if n.submenu:
            for name, href in n.submenu:
                if href:
                    yield T.a(href=href)[name], ' '
                else:
                    yield T.a(_class='disabled')[name], ' '
    
    def getStylesheet(self):
        return '../' * self.context.depth + 'ggi.css'
    
    def getDate(self, ctx, data):
        yield 'built '
        yield datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M UTC")
        yield T.br
        yield 'source '
        yield self.context.navigation.timestamp.strftime("%Y-%m-%d %H:%M UTC")

    def getPrev(self):
        n = self.context.navigation.prev
        if n:
            return T.a(href = n)['prev']
        return 'prev'
    
    def getNext(self):
        n = self.context.navigation.next
        if n:
            return T.a(href = n)['next']
        return 'next'
    
    def getUp(self):
        n = self.context.navigation.up
        if n:
            return T.a(href = n)['up']
        return 'up'

    def getContent(self):
        return T.h1[ 'No content' ]

    def getTitle(self):
        return 'No title'



class ReSTPage(Page):
    sectionLevel = 1
    sectionTag   = T.h1, T.h2, T.h3, T.h4, T.h5, T.h6
    
    noParagraph = ('list_item', 'entry')

    skipUnknownNodes = True
    
    def __init__(self, node, ctx):
        Page.__init__(self, ctx)
        self.root = node
        
    def getContent(self):
        return self.node(self.root)

    def getTitle(self):
        return [ i.astext()
                 for i in get_children(get_children(self.root)[0])
                 if i.tagname == '#text' ]

    def unknown(self, node):
        print "*>> unknown node '%s'"%node.tagname
        if self.skipUnknownNodes:
            return
        yield T.div(style = 'border: 2px solid red; padding: .5em;')[
            self._unknown_node(node)
        ]
    
    def _unknown_nodes(self, nodes):
        for node in nodes:
            yield T.li[ self._unknown_node(node) ]
            
    def _unknown_node(self, node):
        yield T.span(style = "color:blue")[ node.tagname ]
        if node.tagname == '#text':
            yield ': ',node.astext()
        c = get_children(node)
        if c:
            yield T.ul[ self._unknown_nodes(c) ]

    def text(self, node):
        return node.astext().replace('\x00\\', '\\')
    
    def nodes(self, nodes):
        for node in nodes:
            yield self.node(node)
            
    def node(self, node):
        if node.tagname == '#text':
            return self.text(node)
        return getattr(self, 'node_%s'%node.tagname, self.unknown)(node)
    
    def children(self, node):
        for i in get_children(node):
            yield self.node(i)

    def skip(self, node):
        return ''
    
    # main structural elements
    def node_document(self, node):
        yield T.h1[ self.children(get_children(node)[0]) ]
        yield self.children(node)
        
    node_title  = skip
    node_topic  = skip
    
    def node_section(self, node):
        name = node.get('id',None)
        tnode = get_children(node)[0]
        title = tnode.astext()
        if name is None:
            name = '-'.join(title.lower().split())
            if name[0] in '0123456789':
                name = '_' + name
            for i in "':?!/,;.&()[]":
                name = name.replace(i, '-')
        yield self.sectionTag[self.sectionLevel](id=name)[T.a(name=name)[
              self.children(tnode)
              ]]
        self.sectionLevel += 1
        yield T.div(_class='section-%i'%self.sectionLevel)[ self.children(node) ]
        self.sectionLevel -= 1
        
    def node_paragraph(self, node):
        # intercept specials ggi content
        if hasattr(node, 'ggi'):
            return getattr(self, 'ggi_%s'%node.ggi)(node)
        if node.parent.tagname in self.noParagraph and len(get_children(node.parent)) == 1:
            return self.children(node)
        return T.p[ self.children(node) ]
    
    def node_strong(self, node):
        return T.strong[ self.children(node) ]

    def node_emphasis(self, node):
        return T.em[ self.children(node) ]

    def node_note(self, node):
        return T.div(_class='admonition-note')[
            T.div(_class='admonition-title')['Note'],
            self.children(node)
        ]

    def node_important(self, node):
        return T.div(_class='admonition-important')[
            T.div(_class='admonition-title')['Important'],
            self.children(node)
        ]

    def node_tip(self, node):
        return T.div(_class='admonition-tip')[
            T.div(_class='admonition-title')['Tip'],
            self.children(node)
        ]
    
    # links
    def node_reference(self, node):
        href='/'
        if node.has_key('refuri'):
            href = node['refuri']
        elif node.has_key('refid'):
            href = '#' + node['refid']
        elif node.has_key('refname'):
            href = '#' + self.document.nameids[node['refname']]
        else:
            yield T.span(style="color: red")[ 'Unresolved:' ]
        def _():
            for i in 'https://', 'ftp://', 'http://':
                if href.startswith(i):
                    return T.a(href=href, _class='external')[node.astext(),T.span[u'\u25B8']]
            return T.a(href=href)[node.astext()] # ,T.span[u'\u25C2']]
        yield _()
            
    node_target = skip
    
    # docinfo
    def node_docinfo(self, node):
        return T.div(_class='docinfo')[ T.table[ self.children(node)] ]
    
    def node_revision(self, node):
        return T.tr[ T.td(_class='field')['Revision : '],
                     T.td(_class='entry')[ node.astext() ] ]
    def node_author(self, node):
        return T.tr[ T.td(_class='field')['Author : '],
                     T.td(_class='entry')[ node.astext() ] ]
    def node_date(self, node):
        return T.tr[ T.td(_class='field')['Date : '],
                     T.td(_class='entry')[ node.astext() ] ]   
    def node_field(self, node):
        c = get_children(node)
        return T.tr[ T.td(_class='field')[ c[0].astext(),' : ' ],
                     T.td(_class='entry')[ c[1].astext() ] ]
    
    # lists and enums
    def node_bullet_list(self, node):
        return T.ul[ self.children(node) ]
    
    def node_enumerated_list(self, node):
        return T.ol[ self.children(node) ]
    
    def node_list_item(self, node):
        return T.li[ self.children(node) ]
    
    def node_definition_list(self, node):
        return T.dl[ self.children(node) ]
    
    def node_definition_list_item(self, node):
        return self.children(node)
    
    def node_term(self, node):
        return T.dt[ self.children(node) ]
    
    def node_definition(self, node):
        return T.dd[ self.children(node)]

    # table
    def node_table(self, node):
        return T.table(_class='table')[ self.children(node) ]
    def node_colspec(self, node):
        return ''
    def node_thead(self, node):
        self._col = T.th
        yield self.children(node)
        del self._col
    def node_tgroup(self, node):
        return self.children(node)
    def node_tbody(self, node):
        self._col = T.td
        yield self.children(node)
        del self._col
    def node_row(self, node):
        return T.tr[self.children(node)]
    def node_entry(self, node):
        r = self._col
        if node.has_key('morerows'):
            r = r(rowspan = node['morerows']+1)
        if node.has_key('morecols'):
            r = r(colspan = node['morecols']+1)
        return r[self.children(node)]
    
    
    # misc
    def node_substitution_definition(self, node):
        return ''
    
    def node_image(self, node):
        tag = T.img(src=node['uri'])
        for i in 'height', 'width':
            r = node.get(i, None)
            if r: tag = tag(**{i:r})
        tag = tag(alt=node.get('alt', node['uri']))
        return tag
    
    def node_block_quote(self, node):
        return T.div(_class='blockquote')[ self.children(node) ]

    def node_literal_block(self, node):
        return T.pre(_class='literal_block')[ node.astext() ]
    
    def node_literal(self, node):
        if hasattr(node, 'ggirole'):
            return getattr(self, 'ggirole_%s'%node.ggirole)(node)
        return T.span(_class='literal')[ "'",self.children(node),"'" ]
    
    def ggirole_man(self, node):
        cand = self.context.findPage(node.manref[0], node.manref[1])
        if cand:
            c = cand[0]
            href = 'documentation/%s/%s/%s.%s.html'%(c.pkgname, c.branch,
                                                     c.entry, c.volume)
            return T.a(_class='manpage',
                       href='../'*self.context.depth+href)['%s(%s)'%(node.manref[0],
                                                                 c.volume)]
        return T.span(_class='manpage')[self.children(node)]
    
    def ggirole_function(self, node):
        return T.span(_class='function')[ self.children(node) ]
    
    def ggirole_value(self, node):
        return T.span(_class='value')[self.children(node)]
    
    def ggirole_variable(self, node):
        return T.span(_class='variable')[self.children(node)]
    
    def ggirole_default(self, node):
        token = node.astext()
        if hasattr(self.root,'manpage') and token in self.root.manpage.pages:
            return T.span(_class='function')[token]
        cand = self.context.findTerm(token)
        if not cand:
            return T.span(_class='default')[ self.children(node) ]        
        if cand[0].type=='man':
            c = cand[0]
            href = 'documentation/%s/%s/%s.%s.html'%(c.pkgname, c.branch,
                                                     c.entry, c.volume)
            return T.a(_class='manpage',
                       href='../'*self.context.depth+href)['%s(%s)'%(token,
                                                                 c.volume)]
        else:
            return T.a(_class='term')[self.children(node)]

    def ggi_buildMatrix(self, node):
        from ggitools import releng
        return releng.buildMatrix(*releng.buildsAndArchs(
            os.path.join(ggi.repository, 'html/tmp/latestBuilds')))
    
    def ggi_documentationMatrix(self, node):
        
        def _(p):
            yield T.td[
                T.a(href='../'*self.context.depth+'packages/%s.html'%p.pkgname)[
                  p.name ]
                ]
            for rn in ggi.releaseNames:
                if rn in p.releases:
                    yield T.td[ T.a(href='%s/%s/index.html'%(
                        p.pkgname,
                        p.releases[rn]))[p.releases[rn]]]
                else:
                    yield T.td['']
        def __():
            yield T.tr[ T.th['Package'], [
                T.th[ i == 'current' and 'current' or 'GGI %s.%s'%(i[0], i[1])] for i in ggi.releaseNames]]
            last = None
            for p in packages:
                if p.family != last:
                    yield T.tr[T.td[T.strong[p.family]],
                               T.td(colspan=len(ggi.releaseNames))['']]
                yield T.tr[ _(p) ]
                last = p.family
        tools.sortBy(ggi.releases, ggi.byFamily, ggi.byPackage, ggi.byRelease)
        packages = []
        for r in ggi.releases:
            if not packages or packages[-1].pkgname != r.pkgname:
                packages.append(tools.D(pkgname=r.pkgname,
                                        name=r.name,
                                        family=r.family,
                                        releases={}))
            packages[-1].releases[r.release] = r.branch
        return T.table(_class='table')[__()]


class ManpageIndex(ReSTPage):
    
    def node_section(self, node):
        if hasattr(node, 'manpage'):            
            return T.div[ T.a(href=node.manpage.htmlFilename())[
                str(node.manpage.title) ] ]
        return ReSTPage.node_section(self, node)


    def getContent(self):
        yield self.node(self.root)
        yield T.h2['Manpage index']
        
        nb = 4

        r = {}
        for m in self.context.package.manpages:
            if not m.volume in r: r[m.volume] = []
            r[m.volume].extend(m.pages)
        keys = r.keys()
        keys.sort()
        for volume in keys:
            r[volume].sort()
            yield T.h3[ 'Volume ', volume ]
            def _(l):
                n = 0
                for page in l:
                    yield T.td[T.a(href='%s.%s.html'%(page,volume))[page]]
                    n += 1
                while n != nb:
                    yield T.td
                    n += 1
            cols = [ r[volume][nb*i:(nb*i)+nb]
                     for i in xrange(max(len(r[volume])/nb, 1)) ]
            yield T.table(_class='manpages')[[T.tr[_(i)] for i in cols]]
            


def publishManpages(source, destination, gen, ctx):
    if not os.path.isfile(source):
        print '*>> %s: no such file'%source
        return
    if not os.path.isdir(destination):
        os.makedirs(destination)
    document = parser.loadDocument(source)
    
    ctx.navigation = document.navigation
    ctx.navigation.up = '../../index.html'
    ctx.depth = 3
    page = ManpageIndex(document, ctx)
    dest = os.path.join(destination, 'index.html')
    print 'P ', dest
    first = '%s.%s.html'%(ctx.package.manpages[0].pages[0],
                          ctx.package.manpages[0].volume)
    ctx.navigation.submenu = [ ('index', None),
                               ('prev',  None),
                               ('next',  first) ]
    page.writeToFile(dest)
    
    ctx.navigation.up = 'index.html'
    for n, m in enumerate(ctx.package.manpages):
        if (m.pages[0], m.volume) in gen:
            ctx.navigation.next = None
            ctx.navigation.prev = None
            if n > 0:
                np = ctx.package.manpages[n-1]
                href = 'documentation/%s/%s/%s.%s.html'%(ctx.package.pkgname,
                                                         ctx.package.branch,
                                                         np.pages[0],np.volume)
                ctx.navigation.prev = '../'*ctx.depth+href
            if n < len(ctx.package.manpages)-1:
                np = ctx.package.manpages[n+1]
                href = 'documentation/%s/%s/%s.%s.html'%(ctx.package.pkgname,
                                                         ctx.package.branch,
                                                         np.pages[0],np.volume)
                ctx.navigation.next = '../'*ctx.depth+href
            dest = os.path.join(destination,'%s.%s.html'%(m.pages[0],m.volume))
            page = ReSTPage(document.manpages.get(m.pages[0],m.volume), ctx)
            page.sectionLevel = 0
            print 'P ', dest

            ctx.navigation.submenu = [ ('index', ctx.navigation.up),
                                       ('prev',ctx.navigation.prev),
                                       ('next',ctx.navigation.next) ]
            page.writeToFile(dest)
            for p in m.pages[1:]:
                l = os.path.join(destination, '%s.%s.html'%(p,m.volume))
                if os.path.isfile(l) or os.path.islink(l): os.remove(l)
                print 'L ', l, '-> %s.%s.html'%(m.pages[0], m.volume)
                os.symlink('%s.%s.html'%(m.pages[0], m.volume), l)


def publishPage(source, target, ctx):
    import parser, html
    depends = tools.includedFiles(source)
    tgt = os.path.join(ctx.destination, target)
    if tools.needUpdate(tgt, depends):
        document = parser.loadDocument(source)
        ctx.navigation = document.navigation
        ctx.depth = target.count('/')
        ctx.package = None
        page = ReSTPage(document, ctx)
        print 'P ', tgt
        if not os.path.isdir(os.path.dirname(tgt)):
            print 'D ', os.path.dirname(tgt)
            os.makedirs(os.path.dirname(tgt))
        page.writeToFile(tgt)


def publishFile(source, target, ctx):
    target = os.path.join(ctx.destination, target)
    if tools.needUpdate(target, source):
        if not os.path.isdir(os.path.dirname(target)):
            print 'D ', os.path.dirname(target)
            os.makedirs(os.path.dirname(target))
        print 'C ', target
        shutil.copy2(source, target)


def publishSite(ctx):
    from ggitools import releng
    
    def source(filename):
        return os.path.join(ggi.repository, filename)
    
    pages     = [ i.strip() for i in file(source('html/tmp/pages')) ]
    files     = [ i.strip() for i in file(source('html/tmp/files')) ]
    
    publishPage(source('html/tmp/releases'), 'packages/releases.html', ctx)
    releng.publishMatrix(source('html/tmp/latestBuilds'),
                         os.path.join(ctx.destination,'releng'),
                         ctx)
    for i in pages:
        publishPage(source('html/%s'%i), i[:-4]+'.html', ctx)
    for i in files:
        publishFile(source('html/%s'%i), i, ctx)
    publishFile(source('html/infrastructure/ggi.css'), 'ggi.css',ctx)


def publishHTMLDocumentation(context):
    for pkg in context.packages:
        todo, seen, gen = [], [], []
        targetdir = os.path.join(context.destination,
                                 'documentation/%s/%s'%(pkg.pkgname,
                                                        pkg.branch))
        print '>> html documentation for', pkg.pkgname, pkg.branch
        for m in pkg.manpages:
            if not m.filename in seen:
                seen.append(m.filename)
                if not m.filename in todo and tools.needUpdate(
                    os.path.join(targetdir,
                                 '%s.%s.html'%(m.pages[0],
                                               m.volume)),m.filename):
                    todo.append(m.filename)
            if m.filename in todo:
                gen.append((m.pages[0], m.volume))
        context.package = pkg
        if todo or tools.needUpdate(os.path.join(targetdir, 'index.html'),
                                    pkg.docfile):
            publishManpages(pkg.docfile, targetdir, gen, context)
