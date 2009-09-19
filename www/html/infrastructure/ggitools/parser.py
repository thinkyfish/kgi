from docutils.parsers.rst import directives, roles
import docutils.nodes
import os
import datetime

class Navigation:
    """
    Per-page navigation info
    
    document nodes will all have a .navigation attribute
    """
    
    def __init__(self, next=None, prev=None, up=None, menu=None, submenu=None):
        self.next    = next
        self.prev    = prev
        self.up      = up
        self.menu    = [
            ('Home',		  'index.html'),
            ('Contact',		  'contact.html'),
            ('Credits',		  'credits.html'),			
			('Development',   'development.html'),
            ('Documentation', 'documentation.html'),
            ('Download',      'download.html'),
			('Licensing',     'licensing.html'),
			('Links',         'links.html'),
            ('Screenshots',	  'screenshots.html')
          ]
        self.submenu = submenu

class Manpage:
    """A manpage"""
    def __init__(self, title, volume, pages, node = None):
        self.title  = title
        self.volume = volume
        self.pages  = pages
        self.node   = node
        
    def resolve(self, ref):
        return self.volume == ref[1] and ref[0] in self.pages
    
    def htmlFilename(self, prefix=''):
        return '%s%s.%s.html'%(prefix, self.pages[0], self.volume)
    
    def htmlLink(self, name, prefix = ''):
        return '<a href="%s%s.%s.html">%s(%s)</a>'%(
            prefix, self.pages[0], self.volume, name, self.volume)
    def manLink(self):
        return '.so man%s/%s.%s\n'%(self.volume, self.pages[0], self.volume)
    
class Manpages:
    """A Collection of manpages"""
    def __init__(self):
        self.pages   = []
        
    def add(self, title, volume, pages, node = None):
        manpage = Manpage(title, volume, pages, node)
        self.pages.append(manpage)
        return manpage

    def get(self, page, volume):
        for p in self.pages:
            if p.pages[0] == page and p.volume == volume:
                return p.node
        
##################################################################
# GGI specific inline roles
##################################################################

roles.DEFAULT_INTERPRETED_ROLE = 'default'

def role_man(name, rawtext, text, lineno, inliner,
             options={}, content=[]):
    r = docutils.nodes.literal(rawtext, text)
    r.ggirole = 'man'
    r.manref = tuple(text[:-1].split("("))
    if len(r.manref) != 2:
        raise Exception("%s: invalid manpage"%text)
    return [ r ], []

def role_default(name, rawtext, text, lineno, inliner,
                options={}, content=[]):
    r = docutils.nodes.literal(rawtext, text)
    r.ggirole = 'default'
    return [ r ], []

def role_variable(name, rawtext, text, lineno, inliner,
                   options={}, content=[]):
    r = docutils.nodes.literal(rawtext, text)
    r.ggirole = 'variable'
    return [ r ], []

def role_function(name, rawtext, text, lineno, inliner,
                   options={}, content=[]):
    r = docutils.nodes.literal(rawtext, text)
    r.ggirole = 'function'
    return [ r ], []

def role_value(name, rawtext, text, lineno, inliner,
                options={}, content=[]):
    r = docutils.nodes.literal(rawtext, text)
    r.ggirole = 'value'
    return [ r ], []

roles.register_canonical_role('p',      role_variable)
roles.register_canonical_role('v',      role_value)
roles.register_canonical_role('f',      role_function)

roles.register_canonical_role('var',    role_variable)
roles.register_canonical_role('val',    role_value)
roles.register_canonical_role('func',   role_function)

roles.register_canonical_role('man',    role_man)

roles.register_canonical_role('default', role_default)


##################################################################
# GGI specific directives
##################################################################

def self(arg): return arg

def _submenu(arg):
    r = []
    if arg:
        for i in arg.split(','):
            name, href = i.split(':')
            r.append((name.strip(), href.strip()))
    return r

def directive_navigation(name, arguments, options, content, lineno,
                         content_offset, block_text, state, state_machine):
    navigation = state.parent.document.navigation
    for i in ('next', 'prev', 'up', 'submenu'):
        if i in options: setattr(navigation, i, options[i])
    return []
directive_navigation.arguments = (0,0,0)
directive_navigation.options   = {'next':self,
                                  'prev':self,
                                  'up':self,
                                  'keywords':self,
                                  'submenu':_submenu}
directive_navigation.content   = 0

# automagically create a link to the hotfix file
def directive_hotfix(name, arguments, options, content, lineno,
                     content_offset, block_text, state, state_machine):
    date, file = arguments
    # XXX make this configurabel, or part of a SiteMap singleton
    uri = "http://www.ggi-project.org/ftp/patches/hotfixes/%s"%file
    ref = docutils.nodes.reference('', date + " : " +file)
    ref['refuri'] = uri
    hf = docutils.nodes.definition_list()
    dli = docutils.nodes.definition_list_item()
    t = docutils.nodes.term()
    t += ref
    dli += t
    hf += dli
    p = docutils.nodes.definition()
    dli += p
    state.nested_parse(content, content_offset, p)
    return [hf]

directive_hotfix.arguments = (2,0,0)
directive_hotfix.options   = None
directive_hotfix.content   = 1


# Mark the current section as a manpage
def directive_manpage(name, arguments, options, content, lineno,
                      content_offset, block_text, state, state_machine):
    # We had a new attribute to the section with all necessary information
    # It is transparent for other docutils processes
    
    volume, pages   = arguments[0], arguments[1].split()
    # the title of the page is the title of the section we're in
    title = state.parent[0][0]
    
    # add the NAME section
    sect = docutils.nodes.section()
    sect += docutils.nodes.title('','Name')
    p = docutils.nodes.paragraph('', '')
    for i in pages:
        n = docutils.nodes.literal(i,i)
        n.ggirole = 'function'
        p += n
        if not i == pages[-1]:
            p += docutils.nodes.Text(', ')
    p += docutils.nodes.Text(' : %s'%title)
    sect += p
    
    # register the manpages reference at toplevel for :
    if not hasattr(state.parent.document, 'manpages'):
        state.parent.document.manpages = Manpages()
    state.parent.manpage = state.parent.document.manpages.add(title,
                                                              volume,
                                                              pages,
                                                              state.parent)
    # add navigation info to the page
    state.parent.navigation = Navigation()
    state.parent.navigation.timestamp = state.parent.document.navigation.timestamp

    return [ sect ]

directive_manpage.arguments = (2,0,1)
directive_manpage.options = None
directive_manpage.content = 0


def skip_include(name, arguments, options, content, lineno,
                 content_offset, block_text, state, state_machine):
    return []
skip_include.arguments = (1, 0, 1)
skip_include.options = {'literal': directives.flag}

def directive_ggi(name, arguments, options, content, lineno,
                  content_offset, block_text, state, state_machine):
    p = docutils.nodes.paragraph('', '')
    p.ggi = arguments[0]
    return [ p ]
directive_ggi.arguments = (1,0,0)
directive_ggi.options   = None
directive_ggi.content   = 0


# registration
directives.register_directive('hotfix', directive_hotfix)
directives.register_directive('manpage', directive_manpage)
directives.register_directive('navigation', directive_navigation)
directives.register_directive('ggi', directive_ggi)

def loadDocument(filename, ignoreIncludes = False):
    """
    Loads a ReST document with ggi-specifc add-ons.
    
    If ignoreIncludes is true, 'include' directives will be ignored
    """
    import docutils.parsers.rst
    import docutils.utils
    import docutils.transforms
    import docutils.frontend
    import docutils.readers.standalone
    
    if ignoreIncludes:
        directives.register_directive('include', skip_include)
    else:
        # docutils is so lame
        if 'include' in directives._directives:
            del directives._directives['include']
            
    parser   = docutils.parsers.rst.Parser()
    reader   = docutils.readers.standalone.Reader(parser)
    settings = docutils.frontend.OptionParser()
    settings.populate_from_components([parser])
    settings = settings.get_default_values()
    settings.sectsubtitle_xform = 0
    document = docutils.utils.new_document(filename, settings)
    document.navigation = Navigation()
    document.navigation.timestamp = datetime.datetime.utcfromtimestamp(
        os.path.getmtime(filename))
    parser.parse(file(filename).read().decode('latin-1').encode('utf-8'), document)
    transformer = docutils.transforms.Transformer(document)
    transformer.populate_from_components((parser,reader,))
    transformer.apply_transforms()
    
    return document
