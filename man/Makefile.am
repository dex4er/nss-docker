man_MANS = nss-docker.1

EXTRA_DIST = $(man_MANS) nss-docker.pod

nss-docker.1: nss-docker.pod
	eval $(POD2MAN) `head -n1 $(srcdir)/nss-docker.pod | sed 's/^# pod2man //'` $(srcdir)/nss-docker.pod $@
