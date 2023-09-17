oci_tool := `which podman > /dev/null && echo podman || echo docker`
oci_volume_args := if oci_tool == "podman" { '--ignore' } else { '' }
oci_run_args := if oci_tool == "podman" { '--security-opt label=disable' } else { '' }
build_dir := 'cmake-build-default'

_default:
  @just --list

_prep_volumes:
    @{{oci_tool}} volume create {{oci_volume_args}} tcenv-tcroot > /dev/null
    @{{oci_tool}} volume create {{oci_volume_args}} tcenv-build > /dev/null
    @{{oci_tool}} volume create {{oci_volume_args}} tcenv-ccache > /dev/null

_run IMAGE *ARGS: ( build-image IMAGE ) _prep_volumes
    {{oci_tool}} run --rm -it {{oci_run_args}} -v tcenv-ccache:/root/.cache -v tcenv-tcroot:/tcroot -v `pwd`:/proj -v tcenv-build:/proj/{{build_dir}} -w /proj tcenv:{{IMAGE}} {{ARGS}}

# build container image
build-image IMAGE='proj':
    {{oci_tool}} build -f oci/Dockerfile.{{IMAGE}} -t tcenv:{{IMAGE}} oci

# enter default shell of container
shell IMAGE='proj': ( _run IMAGE )

# inspect container image
dive IMAGE='proj': ( build-image IMAGE )
    dive tcenv:{{IMAGE}}

# browse files in container
lf IMAGE='proj': ( _run IMAGE 'lf' )

# build project. ARGS are passed to cmake, i.e. --target XXX
build *ARGS: configure ( _run 'proj' 'cmake --build --preset=default' ARGS )

# configure project. ARGS are passed to cmake, i.e. --fresh
configure *ARGS: ( _run 'proj' 'cmake --preset=default' ARGS )

# run 'tcenv' in specified stage. Example: just run 0 test
run STAGE *ARGS: build ( _run ('stage' + STAGE) (build_dir + '/tcenv') ARGS )
