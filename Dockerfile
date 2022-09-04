# build
FROM node:18.8-alpine3.16 AS build-env
COPY package*.json .
RUN npm i
COPY . .
RUN npm run build

# runtime
FROM node:18.8-alpine3.16
COPY package*.json .
RUN npm i --omit=dev
COPY --from=build-env dist dist
CMD [ "node", "dist/main.js" ]
