FROM node:18-slim

WORKDIR /app

COPY package.json package-lock.json ./
RUN npm install --production --no-audit

COPY . .
RUN npx expo export
EXPOSE 8080

CMD ["node", "server.js"]
