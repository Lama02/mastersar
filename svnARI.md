# SVN depuis l'ARI #

Pour pouvoir utiliser svn dans les locaux de l'ARI il faudra configurer svn. Pour le faire éditer le fichier ~/.subversion/servers

modifier la section `[global]` de façon a avoir quelques choses qui ressemble à

```
[global]
# http-proxy-exceptions = *.exception.com, www.internal-site.org
## By mastersar
http-proxy-host = proxy.ufr-info-p6.jussieu.fr
http-proxy-port = 3128
```

**ATTENTION :** il ne faut pas avoir des espaces au début des lignes