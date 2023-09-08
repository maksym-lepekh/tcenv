oci_tool := 'docker'
build_dir := 'cmake-build-out'
ug_pair := `id -u` + ':' + `id -g`

_default:
  @just --list

# build container image
build-image IMAGE='proj':
    {{oci_tool}} build -f docker/Dockerfile.{{IMAGE}} -t tvenv:{{IMAGE}} docker

_run IMAGE DIR *ARGS: ( build-image IMAGE )
    {{oci_tool}} run -it -u {{ug_pair}} -v ~/tcroot:/tcroot:Z -v ./{{DIR}}:/proj:Z -w /proj tvenv:{{IMAGE}} {{ARGS}}

# enter default shell of container
shell IMAGE='proj': ( _run IMAGE '' )

# inspect container image
dive IMAGE='proj': ( build-image IMAGE )
    dive tvenv:{{IMAGE}}

# browse files in container
lf IMAGE='proj': ( _run IMAGE '' 'lf' )

# build project
build *ARGS: configure ( _run 'proj' '' 'cmake' '--build' build_dir '--parallel' ARGS )

# configure project
configure *ARGS: ( _run 'proj' '' 'cmake' '-B' build_dir '-S' '.' ARGS )

# run 'tcenv' in specified stage
run STAGE *ARGS: build ( _run ('stage' + STAGE) build_dir './tcenv' ARGS )
