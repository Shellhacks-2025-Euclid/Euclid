# Use a Node.js base image with a specified version
FROM node:22.20-alpine

# Copy the entire project to the working directory
COPY . .

# Set the working directory inside the container
WORKDIR /Euclid-Web

# Install dependencies
RUN npm install

# Build the project
RUN npm run build

# Expose the port on which the server will run
EXPOSE 8080

ARG build_id
ARG commit_hash
ENV BUILD_ID=$build_id
ENV COMMIT_HASH=$commit_hash
ENV NODE_ENV production
ENV PORT 8080

# Command to run the server
CMD ["npm", "start"]