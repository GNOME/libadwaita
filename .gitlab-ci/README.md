### Checklist for Updating the Docker images

 - [ ] Update the `${image}.Dockerfile` file with the dependencies
 - [ ] Run `./run-docker.sh build --base ${image} --version ${number}`
 - [ ] Run `./run-docker.sh push --base ${image} --version ${number}`
   once the Docker image is built; you may need to log in by using
   `docker login` or `podman login`
 - [ ] Update the `image` keys in the `.gitlab-ci.yml` file with the new
   image tag
 - [ ] Open a merge request with your changes and let it run
