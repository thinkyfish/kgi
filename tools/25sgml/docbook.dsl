<!DOCTYPE STYLE-SHEET PUBLIC "-//Norman Walsh//DTD Annotated DSSSL Style Sheet V1.3//EN" [
<!ENTITY html.dsl PUBLIC "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN" CDATA dsssl>
<!ENTITY print.dsl PUBLIC "-//Norman Walsh//DOCUMENT DocBook Print Stylesheet//EN" CDATA dsssl>
]>
<?Pub Inc>
<style-sheet>
<title>DocBook V3.0 Document</title>
<doctype pubid="-//Davenport//DTD DocBook V3.0//EN">
<doctype pubid="-//Arbortext//DTD Arbortext DocBk30 XML V1.0//EN">
<doctype pubid="-//ArborText//DTD DocBook V2.4.1-Based Variant V1.1//EN">
<backend name="rtf"  backend="rtf"  fragid="print" default="true" >
<backend name="tex"  backend="tex"  fragid="print">
<backend name="sgml" backend="sgml" fragid="html"  options="-i html">
<style-specification id="print" use="db-print">
<style-specification-body> 

;; something in here, it all comes from the real stylesheet 

</style-specification-body>
</style-specification>
<style-specification id="html" use="db-html">
<style-specification-body> 

;; something in here, it all comes from the real stylesheet

</style-specification-body>
</style-specification>
<external-specification id="db-print" document="print.dsl">
<external-specification id="db-html" document="html.dsl">
</style-sheet>
<?Pub *0000001018>
