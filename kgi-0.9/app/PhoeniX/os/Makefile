
default.exit:: ../os.o

OBJECTS = WaitFor.o	\
	access.o	\
	connection.o	\
	io.o		\
	osinit.o	\
	utils.o		\
	auth.o		\
	mitauth.o	\
	xdmcp.o		\
	decompress.o	\
	xalloc.o

# if X11_EXT_XC_SECURITY
#	OBJECTS_SECURITY = secauth.o

# if !DDXOsColor
	OBJECTS_COLOR = oscolor.o

# if HasXdmAuth
#	OBJECTS_XDMAUTH	= xdmauth.o

# if HasSecureRPC
#	OBJECTS_RPCAUTH = rpcauth.o

# if HasKrb5
#	OBJECTS_KRB5AUTH = krb5auth.o k5encode.o

# if BuildLBX
#	OBJECTS_LBX = lbxio.o


../os.o: $(OBJECTS) $(OBJECTS_COLOR) $(OBJECTS_XDMAUTH) $(OBJECTS_RPCAUTH) \
	$(OBJECTS_KRB5AUTH) $(OBJECTS_LBX) $(OBJECTS_SECURITY)
	$(LD) -r -o $@ $^

#
#	on HPArchitecture, we need hpsocced.o
#	on AmoebaArchitecture, we need iopreader.o
#
