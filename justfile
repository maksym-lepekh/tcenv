oci_tool := `which podman > /dev/null && echo podman || echo docker`

_default:
  @just --list

# build container image
build-image IMAGE='proj':
    {{oci_tool}} build -f docker/Dockerfile.{{IMAGE}} -t tvenv:{{IMAGE}} docker

_prep_volumes:
    @{{oci_tool}} volume create tcenv-tcroot > /dev/null
    @{{oci_tool}} volume create tcenv-build > /dev/null

_run IMAGE *ARGS: ( build-image IMAGE ) _prep_volumes
    {{oci_tool}} run --rm -it -v tcenv-tcroot:/tcroot -v `pwd`:/proj -v tcenv-build:/proj-out -w /proj-out tvenv:{{IMAGE}} {{ARGS}}

# enter default shell of container
shell IMAGE='proj': ( _run IMAGE )

# inspect container image
dive IMAGE='proj': ( build-image IMAGE )
    dive tvenv:{{IMAGE}}

# browse files in container
lf IMAGE='proj': ( _run IMAGE 'lf' )

# build project
build *ARGS: configure ( _run 'proj' 'cmake' '--build' '.' ARGS )

# configure project
configure *ARGS: ( _run 'proj' 'cmake -B . -S ../proj -G Ninja' ARGS )

# run 'tcenv' in specified stage
run STAGE *ARGS: build ( _run ('stage' + STAGE) './tcenv' ARGS )
