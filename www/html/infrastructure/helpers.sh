. $HOME/.ggisettings


cvsUpdatePackage() {
    local _destination;
    local _module;
    local _tag;
    
    _destination=$1;
    _module=$2;
    _tag=$3;
    
    if [ -d $_destination ]; then
	echo ">>" update $_module $_tag in $_destination
	cd $_destination &&
	cvs -q -z6 up -Pd
    else
	echo ">>" checkout $_module $_tag in $_destination
	mkdir -p $_destination/tmp- &&
	cd $_destination/tmp- &&
	cvs -q -z6 -d$GGICVSROOT co $_tag $_module &&
	mv $_module/* $_module/.cvsignore .. &&
	cd .. &&
	rm -rf tmp-
    fi
}

generateSiteResources() {
    # list pages and files
    cd $GGICVS/html
    mkdir -p tmp
    find * -name "*.txt" > tmp/pages
    find resources -type f | grep -v CVS | grep -v .cvsignore > tmp/files
    
    # python resources
    runPython latestNews releasesIndex
    touch -r news.txt tmp/latestNews
    test ! -f tmp/releases && touch tmp/releases
    diff tmp/releases tmp/releases.new > tmp/releases.diff
    test -s tmp/releases.diff && cp tmp/releases.new tmp/releases

    # ensure navigation (sub-menus) is accessible to modified files
    cp *.menu tmp
    
    # autobuild results
    rm -f tmp/latestBuilds.new
    touch tmp/latestBuilds.new
    test ! -f tmp/latestBuilds && touch tmp/latestBuilds
    test -r ${GGIBUILDLOGS}/latestBuilds && cp ${GGIBUILDLOGS}/latestBuilds tmp/latestBuilds.new
    diff tmp/latestBuilds tmp/latestBuilds.new > tmp/latestBuilds.diff
    test -s tmp/latestBuilds.diff && cp tmp/latestBuilds.new tmp/latestBuilds
    return 0
}

fetchBuildLogs() {
    mkdir -p ${GGIBUILDLOGS}tmp
    scp -Cr ${GGICFLOGIN}:${GGICFBUILDLOGS}/ ${GGIBUILDLOGS}tmp/
    if test $? -ne 0; then
	# transfer failed. Maybe host is down
	rm -rf ${GGIBUILDLOGS}tmp
	exit 1
    fi
    find ${GGIBUILDLOGS}tmp/ -name build.log -exec gzip -f \{\} \;
    rm -rf ${GGIBUILDLOGS}
    mv -f ${GGIBUILDLOGS}tmp/ ${GGIBUILDLOGS}/
    cd ${GGIBUILDLOGS}
    for I in `find . -name build.status`; do
	A=`grep Buildstatus $I | awk '{ print $2; }'`
	if [ "x$A" == "x" ]; then A="BUILDING"; fi
	echo $I $A >> latestBuilds
    done
}

runPython () {
    INFRA=$GGICVS/html/infrastructure
    PPATH=$INFRA:$INFRA/depend:$PYTHONPATH
    PYTHON=`which python2.3 2>/dev/null`
    PYTHON=${PYTHON:-python}
    GGIUSETREE=$GGIUSETREE GGIHTML=$GGIHTML GGIPACKAGES=$GGIPACKAGES GGICVS=$GGICVS PYTHONPATH=$PPATH GGITIMESTAMPFILES=$GGITIMESTAMPFILES $PYTHON $INFRA/ggitools/ggi.py $*
}

updateCVS () {
    mkdir -p $GGICVS/html/tmp
    cd $GGICVS
    for I in html tools; do cd $I; cvs -q up -PAd; cd ..; done
    runPython cvsup
    sh $GGICVS/html/tmp/cvsup.sh
}

updateSite() {
    generateSiteResources && runPython site
}

updateManpages() {
    runPython manpages
}
