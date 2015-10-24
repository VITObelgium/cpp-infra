#include <Eigen/Dense>

int main( int argc, char** argv ) {

  Eigen::MatrixXd A = Eigen::MatrixXd::Random(4000,5000);
  Eigen::MatrixXd B = Eigen::MatrixXd::Random(5000,4000);

  Eigen::MatrixXd C = A * B;

  return 0;
}
