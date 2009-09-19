ggi?=`pwd`/../..

tools=http://www.ggi-project.org/ftp/develtools/sitedep.tgz
infrastructure=${ggi}/html/infrastructure
script=${infrastructure}/ggitools/site.py

all: site

clean:
	rm -rf ${ggi}/html/tmp

site:
	sh -c ". ./helpers.sh; updateSite"

manpages:
	sh -c ". ./helpers.sh; updateManpages;"

update-cvs:
	sh -c ". ./helpers.sh; updateCVS"
