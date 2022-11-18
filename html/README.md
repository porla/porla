# A web UI for Porla

## Getting started

 * Download the latest release archive.
 * Extract the archive somehere, for example `/var/www/porla`.
 * Set up a web server to serve this directory and also proxy all calls to `/api`
   to the Porla backend.

### Nginx

This is a simple Nginx config that takes care of serving the static files as
well as reverse proxying the `/api` calls to the Porla daemon.

```
server {
  listen 80;
  root /var/www/porla;

  location / {
    try_files $uri $uri/ $uri.html /index.html;
  }

  location /api {
    proxy_pass http://localhost:1337;
  }
}
```
