import os
import time

from ggitools import parser
from ggitools import tools

def get_children(node):
    if hasattr(node, 'get_children'):
        return node.get_children()
    return node.children

def raw(s):
    return s

class Page(object):
    
    sectionLevel = -1
    sectionTag   = '.SH %s\n', '.SS %s\n', '\n????? %s\n'
    
    def __init__(self, root, context):
        self.rootNode = root
        self.context = context
        self.stack = []


    def writeToFile(self, dest):
        f = file(dest, 'wb')
        
        def write(thing):
            if isinstance(thing, unicode):
                f.write(thing.encode('utf-8'))
            elif isinstance(thing, str):
                f.write(thing)
            else:
                for c in thing:
                    write(c)

        write(self.render())
        f.close()

#     def writeToFile(self, dest):
#         f = file(dest, 'wb')
#         things = [ self.render() ]
#         while things:
#             thing = things.pop(0)
#             if isinstance(thing, str):
#                 f.write(thing)
#             else:
#                 l = list(thing)
#                 things = l + things
#         f.close()
        
    def render(self):
        yield '.TH "%s" %s "%s" "%s" GGI\n'%(
            self.rootNode.manpage.pages[0], self.rootNode.manpage.volume,
            self.rootNode.navigation.timestamp.strftime("%Y-%m-%d"),
            '%s-%s'%(self.context.package.pkgname,
                     self.context.package.branch))
        yield self.node(self.rootNode)
        
    def text(self, node):
        return raw(node.astext())
    
    def nodes(self, nodes):
        for node in nodes:
            yield self.node(node)
    
    def node(self, node):
        if node.tagname == '#text':
            return self.text(node)
        return getattr(self, 'node_%s'%node.tagname, self.unknown)(node)
    
    def children(self, node):
        for n in get_children(node):
            yield self.node(n)
            
    def skip(self, node):
        return ''
    
    def unknown(self, node):
        print "Warning: unknown node '%s'"%node.tagname
        return ''
    
    # main structural elements
    
    node_document  = children
    node_title     = skip
    node_topic     = skip
    
    def node_section(self, node):
        if self.sectionLevel >= 0:
            title = get_children(node)[0].astext().upper()
            yield raw(self.sectionTag[self.sectionLevel]%title)
        self.sectionLevel += 1
        yield self.children(node)
        self.sectionLevel -= 1
        
    def node_paragraph(self, node):
        import docutils.nodes
        yield self.children(node)
        yield '\n'
        extra = [ docutils.nodes.paragraph,
                  docutils.nodes.literal_block ]
        # add a second \n if next sibling is a paragraph or a section
        try:
            n = node.parent[node.parent.index(node)+1]
            if extra.count(n.__class__): yield '\n'
        except:
            # last node
            pass
        
    def node_strong(self, node):
        return raw('\\fB%s\\fR'%node.astext())
    
    def node_emphasis(self, node):
        return raw('\\fI%s\\fR'%node.astext())

    def node_note(self, node):
        yield '.RS\n'
        yield '\\fBNote:\\fR\n'
        yield self.children(node)
        yield '.RE\n'
        
    def node_important(self, node):
        yield '.RS\n'
        yield '\\fBImportant:\\fR\n'
        yield self.children(node)
        yield '.RE\n'
        
    def node_tip(self, node):
        yield '.RS\n'
        yield '\\fBTip:\\fR\n'
        yield self.children(node)
        yield '.RE\n'
        
    # links
    node_reference = children
    node_target = skip
    
    # docinfo
    node_docinfo = skip
    
    # lists and enums
    def node_bullet_list(self, node):
        yield self.enter_list('\\(bu')
        yield self.children(node)
        yield self.leave_list()
        
    def node_enumerated_list(self, node):
        yield self.enter_list(0)
        yield self.children(node)
        yield self.leave_list()
        
    def node_definition_list(self, node):
        yield self.enter_list(None)
        yield self.children(node)
        yield self.leave_list()

    def node_list_item(self, node):
        yield self.enter_li()
        if isinstance(self.stack[-1][0],int):
            self.stack[-1][0] += 1
            yield '.IP %i 4\n'%self.stack[-1][0]
        elif isinstance(self.stack[-1][0],str):
            yield '.IP %s 4\n'%self.stack[-1][0]
        else:
            print 'Warning: strange list item'
        yield self.children(node)
        yield self.leave_li()
    
    def node_definition_list_item(self, node):
        yield '.TP\n'
        yield self.enter_li()
        yield self.children(node)
        yield self.leave_li()
        
    node_term = children
    
    def node_definition(self, node):
        yield '\n'
        yield self.children(node)
        yield '\n'

    def enter_list(self, t):
        # if we're in a list, that has not been
        # indented yet, we force an indent
        if len(self.stack) and self.stack[-1][1] == 0:
            self.stack[-1][1] = 1
            yield '.RS\n'
        self.stack.append([t, 0])
        
    def leave_list(self):
        # back to normal para flow
        #if self.stack:
        del self.stack[-1]
        return '.PP\n'
    
    def enter_li(self):
        return ''
        
    def leave_li(self):
        # if we have been forced to indent, indent back
        if self.stack[-1][1]:
            yield '.RE\n'
        self.stack[-1][1] = 0
        
    # misc
    node_image = skip
    
    def node_block_quote(self, node):
        yield '.RS\n'
        yield self.children(node)
        yield '.RE\n'
        
    def node_literal_block(self, node):
        yield '.nb\n.nf\n'
        for n in node:
            yield raw(n.astext().replace('\\', '\\e'))
        yield '\n.fi\n\n'
        
    def node_literal(self, node):
        if hasattr(node, 'ggirole'):
            return getattr(self, 'ggirole_%s'%node.ggirole)(node)
        # return raw("\\fB'%s'\\fR"%node.astext())
        return raw("\\f(CW%s\\fR"%node.astext())
    
    def ggirole_function(self, node):
        return raw('\\fB%s\\fR'%node.astext())
    
    def ggirole_variable(self, node):
        return raw('\\fI%s\\fR'%node.astext())
    
    def ggirole_value(self, node):
        return raw('\\fB%s\\fR'%node.astext())

    def ggirole_man(self, node):
        return raw('\\f(CW%s\\fR'%node.astext())
    
    def ggirole_default(self, node):
        token = node.astext()
        if token in self.rootNode.manpage.pages:
            #print "Role: '%s' is manpage entry"%token
            return raw('\\fB%s\\fR'%token)
        cand = self.context.findTerm(token)
        if not cand:
            #print "Warning: unknown term '%s'"%token
            return raw('\\fB%s\\fR'%token)
        if cand[0].type == 'man':
            #print "Role: '%s' is manpage %s(%s)"%(token, page, volume)
            return raw('\\fB%s(%s)\\fR'%(token, cand[0].volume))
        else:
            #print "Role: '%s' is a term"%token
            return raw('\\fB%s\\fR'%token)
    
def publishManpages(source, target, context):
    """
    Publish all manpages found in source in destination directory.
    """
    if not os.path.isfile(source):
        print 'Warning: "%s": no such file'%source
        return
    document = parser.loadDocument(source, ignoreIncludes=True)
    for man in document.manpages.pages:
        dest = os.path.join(target, '%s.%s'%(man.pages[0], man.volume))
        page = Page(man.node, context)
        print 'P ', dest
        page.writeToFile(dest)
        for p in man.pages[1:]:
            print 'L  %s.%s -> %s.%s'%(p,man.volume, man.pages[0], man.volume)
            file(os.path.join(target, '%s.%s'%(p,man.volume)), 'w').write(
                '.so man%s/%s.%s\n'%(man.volume, man.pages[0], man.volume))


def publishMakefile(target):
    """Create a Makefile for all manpages found in *.man files."""
    mf = file(os.path.join(target, 'Makefile.am'), 'w')
    mf.write('MAINTAINERCLEANFILES = Makefile.in\n\n')
    mf.write('man_MANS =')
    for f in [ file(os.path.join(target,i))
               for i in os.listdir(target) if i.endswith('.man') ]:
        for l in f:
            mf.write(' \\\n           %s'%(l.strip()))
    mf.write('\n\nEXTRA_DIST = $(man_MANS)\n')


def updateManpages(context):
    for pkg in context.packages:
        todo, seen = [], []
        targetdir = os.path.join(pkg.directory, 'doc/man')
        print '>> manpages for %s-%s in %s'%(pkg.pkgname,
                                             pkg.branch,
                                             targetdir)
        if not os.path.isdir(targetdir):
            print '*> directory does not exist:', targetdir
            continue
        f = file(os.path.join(targetdir, '%s.man'%pkg.pkgname),'w')
        for m in pkg.manpages:
            if not m.filename in seen:
                seen.append(m.filename)
                if not m.filename in todo and tools.needUpdate(
                    os.path.join(targetdir, '%s.%s'%(m.pages[0], m.volume)),
                    m.filename):
                    todo.append(m.filename)
            for p in m.pages:
                f.write('%s.%s\n'%(p,m.volume))
        f.close()
        context.package = pkg
        for filename in todo:
            publishManpages(filename, targetdir, context)
        publishMakefile(os.path.join(pkg.directory, 'doc/man'))
