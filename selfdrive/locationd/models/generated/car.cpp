#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_8643812751273266633) {
   out_8643812751273266633[0] = delta_x[0] + nom_x[0];
   out_8643812751273266633[1] = delta_x[1] + nom_x[1];
   out_8643812751273266633[2] = delta_x[2] + nom_x[2];
   out_8643812751273266633[3] = delta_x[3] + nom_x[3];
   out_8643812751273266633[4] = delta_x[4] + nom_x[4];
   out_8643812751273266633[5] = delta_x[5] + nom_x[5];
   out_8643812751273266633[6] = delta_x[6] + nom_x[6];
   out_8643812751273266633[7] = delta_x[7] + nom_x[7];
   out_8643812751273266633[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_5143440262880779737) {
   out_5143440262880779737[0] = -nom_x[0] + true_x[0];
   out_5143440262880779737[1] = -nom_x[1] + true_x[1];
   out_5143440262880779737[2] = -nom_x[2] + true_x[2];
   out_5143440262880779737[3] = -nom_x[3] + true_x[3];
   out_5143440262880779737[4] = -nom_x[4] + true_x[4];
   out_5143440262880779737[5] = -nom_x[5] + true_x[5];
   out_5143440262880779737[6] = -nom_x[6] + true_x[6];
   out_5143440262880779737[7] = -nom_x[7] + true_x[7];
   out_5143440262880779737[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_2442215390598688674) {
   out_2442215390598688674[0] = 1.0;
   out_2442215390598688674[1] = 0;
   out_2442215390598688674[2] = 0;
   out_2442215390598688674[3] = 0;
   out_2442215390598688674[4] = 0;
   out_2442215390598688674[5] = 0;
   out_2442215390598688674[6] = 0;
   out_2442215390598688674[7] = 0;
   out_2442215390598688674[8] = 0;
   out_2442215390598688674[9] = 0;
   out_2442215390598688674[10] = 1.0;
   out_2442215390598688674[11] = 0;
   out_2442215390598688674[12] = 0;
   out_2442215390598688674[13] = 0;
   out_2442215390598688674[14] = 0;
   out_2442215390598688674[15] = 0;
   out_2442215390598688674[16] = 0;
   out_2442215390598688674[17] = 0;
   out_2442215390598688674[18] = 0;
   out_2442215390598688674[19] = 0;
   out_2442215390598688674[20] = 1.0;
   out_2442215390598688674[21] = 0;
   out_2442215390598688674[22] = 0;
   out_2442215390598688674[23] = 0;
   out_2442215390598688674[24] = 0;
   out_2442215390598688674[25] = 0;
   out_2442215390598688674[26] = 0;
   out_2442215390598688674[27] = 0;
   out_2442215390598688674[28] = 0;
   out_2442215390598688674[29] = 0;
   out_2442215390598688674[30] = 1.0;
   out_2442215390598688674[31] = 0;
   out_2442215390598688674[32] = 0;
   out_2442215390598688674[33] = 0;
   out_2442215390598688674[34] = 0;
   out_2442215390598688674[35] = 0;
   out_2442215390598688674[36] = 0;
   out_2442215390598688674[37] = 0;
   out_2442215390598688674[38] = 0;
   out_2442215390598688674[39] = 0;
   out_2442215390598688674[40] = 1.0;
   out_2442215390598688674[41] = 0;
   out_2442215390598688674[42] = 0;
   out_2442215390598688674[43] = 0;
   out_2442215390598688674[44] = 0;
   out_2442215390598688674[45] = 0;
   out_2442215390598688674[46] = 0;
   out_2442215390598688674[47] = 0;
   out_2442215390598688674[48] = 0;
   out_2442215390598688674[49] = 0;
   out_2442215390598688674[50] = 1.0;
   out_2442215390598688674[51] = 0;
   out_2442215390598688674[52] = 0;
   out_2442215390598688674[53] = 0;
   out_2442215390598688674[54] = 0;
   out_2442215390598688674[55] = 0;
   out_2442215390598688674[56] = 0;
   out_2442215390598688674[57] = 0;
   out_2442215390598688674[58] = 0;
   out_2442215390598688674[59] = 0;
   out_2442215390598688674[60] = 1.0;
   out_2442215390598688674[61] = 0;
   out_2442215390598688674[62] = 0;
   out_2442215390598688674[63] = 0;
   out_2442215390598688674[64] = 0;
   out_2442215390598688674[65] = 0;
   out_2442215390598688674[66] = 0;
   out_2442215390598688674[67] = 0;
   out_2442215390598688674[68] = 0;
   out_2442215390598688674[69] = 0;
   out_2442215390598688674[70] = 1.0;
   out_2442215390598688674[71] = 0;
   out_2442215390598688674[72] = 0;
   out_2442215390598688674[73] = 0;
   out_2442215390598688674[74] = 0;
   out_2442215390598688674[75] = 0;
   out_2442215390598688674[76] = 0;
   out_2442215390598688674[77] = 0;
   out_2442215390598688674[78] = 0;
   out_2442215390598688674[79] = 0;
   out_2442215390598688674[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_556127903886361245) {
   out_556127903886361245[0] = state[0];
   out_556127903886361245[1] = state[1];
   out_556127903886361245[2] = state[2];
   out_556127903886361245[3] = state[3];
   out_556127903886361245[4] = state[4];
   out_556127903886361245[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_556127903886361245[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_556127903886361245[7] = state[7];
   out_556127903886361245[8] = state[8];
}
void F_fun(double *state, double dt, double *out_7549809492162425065) {
   out_7549809492162425065[0] = 1;
   out_7549809492162425065[1] = 0;
   out_7549809492162425065[2] = 0;
   out_7549809492162425065[3] = 0;
   out_7549809492162425065[4] = 0;
   out_7549809492162425065[5] = 0;
   out_7549809492162425065[6] = 0;
   out_7549809492162425065[7] = 0;
   out_7549809492162425065[8] = 0;
   out_7549809492162425065[9] = 0;
   out_7549809492162425065[10] = 1;
   out_7549809492162425065[11] = 0;
   out_7549809492162425065[12] = 0;
   out_7549809492162425065[13] = 0;
   out_7549809492162425065[14] = 0;
   out_7549809492162425065[15] = 0;
   out_7549809492162425065[16] = 0;
   out_7549809492162425065[17] = 0;
   out_7549809492162425065[18] = 0;
   out_7549809492162425065[19] = 0;
   out_7549809492162425065[20] = 1;
   out_7549809492162425065[21] = 0;
   out_7549809492162425065[22] = 0;
   out_7549809492162425065[23] = 0;
   out_7549809492162425065[24] = 0;
   out_7549809492162425065[25] = 0;
   out_7549809492162425065[26] = 0;
   out_7549809492162425065[27] = 0;
   out_7549809492162425065[28] = 0;
   out_7549809492162425065[29] = 0;
   out_7549809492162425065[30] = 1;
   out_7549809492162425065[31] = 0;
   out_7549809492162425065[32] = 0;
   out_7549809492162425065[33] = 0;
   out_7549809492162425065[34] = 0;
   out_7549809492162425065[35] = 0;
   out_7549809492162425065[36] = 0;
   out_7549809492162425065[37] = 0;
   out_7549809492162425065[38] = 0;
   out_7549809492162425065[39] = 0;
   out_7549809492162425065[40] = 1;
   out_7549809492162425065[41] = 0;
   out_7549809492162425065[42] = 0;
   out_7549809492162425065[43] = 0;
   out_7549809492162425065[44] = 0;
   out_7549809492162425065[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_7549809492162425065[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_7549809492162425065[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_7549809492162425065[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_7549809492162425065[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_7549809492162425065[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_7549809492162425065[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_7549809492162425065[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_7549809492162425065[53] = -9.8000000000000007*dt;
   out_7549809492162425065[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_7549809492162425065[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_7549809492162425065[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7549809492162425065[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7549809492162425065[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_7549809492162425065[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_7549809492162425065[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_7549809492162425065[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_7549809492162425065[62] = 0;
   out_7549809492162425065[63] = 0;
   out_7549809492162425065[64] = 0;
   out_7549809492162425065[65] = 0;
   out_7549809492162425065[66] = 0;
   out_7549809492162425065[67] = 0;
   out_7549809492162425065[68] = 0;
   out_7549809492162425065[69] = 0;
   out_7549809492162425065[70] = 1;
   out_7549809492162425065[71] = 0;
   out_7549809492162425065[72] = 0;
   out_7549809492162425065[73] = 0;
   out_7549809492162425065[74] = 0;
   out_7549809492162425065[75] = 0;
   out_7549809492162425065[76] = 0;
   out_7549809492162425065[77] = 0;
   out_7549809492162425065[78] = 0;
   out_7549809492162425065[79] = 0;
   out_7549809492162425065[80] = 1;
}
void h_25(double *state, double *unused, double *out_4860319073032555070) {
   out_4860319073032555070[0] = state[6];
}
void H_25(double *state, double *unused, double *out_7696968578362287874) {
   out_7696968578362287874[0] = 0;
   out_7696968578362287874[1] = 0;
   out_7696968578362287874[2] = 0;
   out_7696968578362287874[3] = 0;
   out_7696968578362287874[4] = 0;
   out_7696968578362287874[5] = 0;
   out_7696968578362287874[6] = 1;
   out_7696968578362287874[7] = 0;
   out_7696968578362287874[8] = 0;
}
void h_24(double *state, double *unused, double *out_2369380713503777041) {
   out_2369380713503777041[0] = state[4];
   out_2369380713503777041[1] = state[5];
}
void H_24(double *state, double *unused, double *out_6003266168380754708) {
   out_6003266168380754708[0] = 0;
   out_6003266168380754708[1] = 0;
   out_6003266168380754708[2] = 0;
   out_6003266168380754708[3] = 0;
   out_6003266168380754708[4] = 1;
   out_6003266168380754708[5] = 0;
   out_6003266168380754708[6] = 0;
   out_6003266168380754708[7] = 0;
   out_6003266168380754708[8] = 0;
   out_6003266168380754708[9] = 0;
   out_6003266168380754708[10] = 0;
   out_6003266168380754708[11] = 0;
   out_6003266168380754708[12] = 0;
   out_6003266168380754708[13] = 0;
   out_6003266168380754708[14] = 1;
   out_6003266168380754708[15] = 0;
   out_6003266168380754708[16] = 0;
   out_6003266168380754708[17] = 0;
}
void h_30(double *state, double *unused, double *out_6454167261116240129) {
   out_6454167261116240129[0] = state[4];
}
void H_30(double *state, double *unused, double *out_7826307525505527944) {
   out_7826307525505527944[0] = 0;
   out_7826307525505527944[1] = 0;
   out_7826307525505527944[2] = 0;
   out_7826307525505527944[3] = 0;
   out_7826307525505527944[4] = 1;
   out_7826307525505527944[5] = 0;
   out_7826307525505527944[6] = 0;
   out_7826307525505527944[7] = 0;
   out_7826307525505527944[8] = 0;
}
void h_26(double *state, double *unused, double *out_7069376089329770656) {
   out_7069376089329770656[0] = state[7];
}
void H_26(double *state, double *unused, double *out_7008272176473207518) {
   out_7008272176473207518[0] = 0;
   out_7008272176473207518[1] = 0;
   out_7008272176473207518[2] = 0;
   out_7008272176473207518[3] = 0;
   out_7008272176473207518[4] = 0;
   out_7008272176473207518[5] = 0;
   out_7008272176473207518[6] = 0;
   out_7008272176473207518[7] = 1;
   out_7008272176473207518[8] = 0;
}
void h_27(double *state, double *unused, double *out_7187383483263147400) {
   out_7187383483263147400[0] = state[3];
}
void H_27(double *state, double *unused, double *out_8445673236403598761) {
   out_8445673236403598761[0] = 0;
   out_8445673236403598761[1] = 0;
   out_8445673236403598761[2] = 0;
   out_8445673236403598761[3] = 1;
   out_8445673236403598761[4] = 0;
   out_8445673236403598761[5] = 0;
   out_8445673236403598761[6] = 0;
   out_8445673236403598761[7] = 0;
   out_8445673236403598761[8] = 0;
}
void h_29(double *state, double *unused, double *out_3089080184983775536) {
   out_3089080184983775536[0] = state[1];
}
void H_29(double *state, double *unused, double *out_7316076181191135760) {
   out_7316076181191135760[0] = 0;
   out_7316076181191135760[1] = 1;
   out_7316076181191135760[2] = 0;
   out_7316076181191135760[3] = 0;
   out_7316076181191135760[4] = 0;
   out_7316076181191135760[5] = 0;
   out_7316076181191135760[6] = 0;
   out_7316076181191135760[7] = 0;
   out_7316076181191135760[8] = 0;
}
void h_28(double *state, double *unused, double *out_8169467576598445512) {
   out_8169467576598445512[0] = state[0];
}
void H_28(double *state, double *unused, double *out_8695940781099373979) {
   out_8695940781099373979[0] = 1;
   out_8695940781099373979[1] = 0;
   out_8695940781099373979[2] = 0;
   out_8695940781099373979[3] = 0;
   out_8695940781099373979[4] = 0;
   out_8695940781099373979[5] = 0;
   out_8695940781099373979[6] = 0;
   out_8695940781099373979[7] = 0;
   out_8695940781099373979[8] = 0;
}
void h_31(double *state, double *unused, double *out_8497768601286577420) {
   out_8497768601286577420[0] = state[8];
}
void H_31(double *state, double *unused, double *out_7666322616485327446) {
   out_7666322616485327446[0] = 0;
   out_7666322616485327446[1] = 0;
   out_7666322616485327446[2] = 0;
   out_7666322616485327446[3] = 0;
   out_7666322616485327446[4] = 0;
   out_7666322616485327446[5] = 0;
   out_7666322616485327446[6] = 0;
   out_7666322616485327446[7] = 0;
   out_7666322616485327446[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_8643812751273266633) {
  err_fun(nom_x, delta_x, out_8643812751273266633);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_5143440262880779737) {
  inv_err_fun(nom_x, true_x, out_5143440262880779737);
}
void car_H_mod_fun(double *state, double *out_2442215390598688674) {
  H_mod_fun(state, out_2442215390598688674);
}
void car_f_fun(double *state, double dt, double *out_556127903886361245) {
  f_fun(state,  dt, out_556127903886361245);
}
void car_F_fun(double *state, double dt, double *out_7549809492162425065) {
  F_fun(state,  dt, out_7549809492162425065);
}
void car_h_25(double *state, double *unused, double *out_4860319073032555070) {
  h_25(state, unused, out_4860319073032555070);
}
void car_H_25(double *state, double *unused, double *out_7696968578362287874) {
  H_25(state, unused, out_7696968578362287874);
}
void car_h_24(double *state, double *unused, double *out_2369380713503777041) {
  h_24(state, unused, out_2369380713503777041);
}
void car_H_24(double *state, double *unused, double *out_6003266168380754708) {
  H_24(state, unused, out_6003266168380754708);
}
void car_h_30(double *state, double *unused, double *out_6454167261116240129) {
  h_30(state, unused, out_6454167261116240129);
}
void car_H_30(double *state, double *unused, double *out_7826307525505527944) {
  H_30(state, unused, out_7826307525505527944);
}
void car_h_26(double *state, double *unused, double *out_7069376089329770656) {
  h_26(state, unused, out_7069376089329770656);
}
void car_H_26(double *state, double *unused, double *out_7008272176473207518) {
  H_26(state, unused, out_7008272176473207518);
}
void car_h_27(double *state, double *unused, double *out_7187383483263147400) {
  h_27(state, unused, out_7187383483263147400);
}
void car_H_27(double *state, double *unused, double *out_8445673236403598761) {
  H_27(state, unused, out_8445673236403598761);
}
void car_h_29(double *state, double *unused, double *out_3089080184983775536) {
  h_29(state, unused, out_3089080184983775536);
}
void car_H_29(double *state, double *unused, double *out_7316076181191135760) {
  H_29(state, unused, out_7316076181191135760);
}
void car_h_28(double *state, double *unused, double *out_8169467576598445512) {
  h_28(state, unused, out_8169467576598445512);
}
void car_H_28(double *state, double *unused, double *out_8695940781099373979) {
  H_28(state, unused, out_8695940781099373979);
}
void car_h_31(double *state, double *unused, double *out_8497768601286577420) {
  h_31(state, unused, out_8497768601286577420);
}
void car_H_31(double *state, double *unused, double *out_7666322616485327446) {
  H_31(state, unused, out_7666322616485327446);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
