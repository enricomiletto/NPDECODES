#if SOLUTION
# Dependencies of mastersolution:
#else
# Add your custom dependencies here:
#endif

# DIR will be provided by the calling file.

set(SOURCES
  ${DIR}/stableevaluationatapoint_main.cc
  ${DIR}/stableevaluationatapoint.h
  ${DIR}/stableevaluationatapoint.cc
)

set(LIBRARIES
  Eigen3::Eigen
  LF::lf.base
  LF::lf.io
  LF::lf.geometry
  LF::lf.mesh.hybrid2d
  LF::lf.mesh.utils
  LF::lf.refinement
  LF::lf.uscalfe
)
