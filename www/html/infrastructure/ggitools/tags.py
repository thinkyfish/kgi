class Tag(object):

    def __init__(self, name):
        self.name = name
        self.attributes = {}
        self.children = []

    def clone(self):
        r = Tag(self.name)
        r.attributes.update(self.attributes)
        r.children.extend(self.children)
        return r
    
    def __call__(self, **args):
        t = self.clone()
        def clean(k):
            if k.startswith('_'):
                k = k[1:]
            if k.endswith('_'):
                k = k[:-1]
            return k
        if args:
            for k, v in args.iteritems():
                t.attributes[ clean(k) ] = v

        return t

    def __getitem__(self, *items):
        t = self.clone()
        t.children.extend(items)
        return t

class raw(object):
    def __init__(self, content):
        self.content = content


tagNames = """
html head title script link style
body a br dd div dl dt em  h1 h2 h3 h4 h5 h6 img li meta ol p pre
span strong table thead tbody td th tr ul
""".strip().split()

for tag in tagNames:
    globals()[tag] = Tag(tag)
