#ifndef __ARM_H
#define __ARM_H

#include "include.h"
#include "BallJoint.h"

#define DEBUG 0

class Arm {
 public:
 	Arm() : base(0.0, 0.0, 0.0) {}
 	T getLength() {
 		T len = 0.0;
 		for (auto &joint : joints) {
 			len += joint->length;
 		}
 		return len;
 	}

 	Vector3f getEndEffector(int num_joint) {
 		Vector3f end = base;
 		for (int i = 0; i < num_joint; i++) {
 			end += joints[i]->get_end();
 		}
 		return end;
 	}

 	Vector3f getEndEffector() {
 		return getEndEffector((int)joints.size());
 	}

 	void draw() {
 		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 		glShadeModel(GL_SMOOTH);

 		Vector3f start = base;
 		for (auto &joint: joints) {
 			start = joint->draw(start);
 		}
 	}

 	template<typename Mat>
 	Mat pseudoInverse(const Mat &m, T eps = 1e-8) {
 		JacobiSVD<Mat> svd(m, Eigen::ComputeThinU | Eigen::ComputeThinV);
 		eps = eps * max(m.cols(), m.rows()) * svd.singularValues().array().abs()(0);
 		return svd.matrixV() * (svd.singularValues().array().abs() > eps).select(
 			svd.singularValues().array().inverse(), 0).matrix().asDiagonal() *
 			svd.matrixU().adjoint();
 	}

 	void solve(Vector3f goal) {
 		T prev_err, cur_err;
 		int max_iter = 200, iter = 0;
 		T err_eps = 1e-2;

 		goal -= base;
 		T maxlen = getLength();
 		if (goal.norm() > maxlen) {
 			goal = goal.normalized() * maxlen;
 		}

 		Vector3f cur = getEndEffector();
 		cur_err = prev_err = (goal - cur).norm();
 		int num_joints = (int)joints.size();

 		while (iter < max_iter && cur_err > err_eps) {
 			Vector3f diff = goal - cur;
 			MatrixXf jac_t(3 * num_joints, 3);

 			for (int i = 0; i < 3 * num_joints; i += 3) {
 				Matrix<T, 1, 3> row_x = compute_joint_jacobian(i/3, goal, joints[i/3]->get_x());
 				Matrix<T, 1, 3> row_y = compute_joint_jacobian(i/3, goal, joints[i/3]->get_y());
 				Matrix<T, 1, 3> row_z = compute_joint_jacobian(i/3, goal, joints[i/3]->get_z());

 				for (int j = 0; j < 3; j++) {
 					jac_t(i,     j) = row_x(0, j);
 					jac_t(i + 1, j) = row_y(0, j);
 					jac_t(i + 2, j) = row_z(0, j);
 				}
 			}

 			MatrixXf jac(3, 3 * num_joints);
 			jac = jac_t.transpose();

 			MatrixXf pseudoinv_jac(3 * num_joints, 3);
 			pseudoinv_jac = pseudoInverse(jac);

 			Matrix<T, Dynamic, 1> delta = pseudoinv_jac * diff;

 			for (int i = 0; i < 3 * num_joints; i += 3) {
 				joints[i/3]->save_transformation();
 				joints[i/3]->apply_delta(delta[i], joints[i/3]->get_x());
 				joints[i/3]->apply_delta(delta[i+1], joints[i/3]->get_y());
 				joints[i/3]->apply_delta(delta[i+2], joints[i/3]->get_z());
 			}

 			cur = getEndEffector();
 			prev_err = cur_err;
 			cur_err = (goal - cur).norm();

 			for (int halve_iter = 0; halve_iter < 100 && cur_err > prev_err; halve_iter++) {
 				for (int i = 0; i < num_joints; i++) {
 					joints[i]->load_transformation();
 				}

 				delta *= 0.5;
 				for (int i = 0; i < 3 * num_joints; i += 3) {
 					joints[i/3]->save_transformation();
 					joints[i/3]->apply_delta(delta[i], joints[i/3]->get_x());
	 				joints[i/3]->apply_delta(delta[i+1], joints[i/3]->get_y());
	 				joints[i/3]->apply_delta(delta[i+2], joints[i/3]->get_z());
 				}
 				cur = getEndEffector();
 				cur_err = (goal - cur).norm();
 			}
 			#if DEBUG == 1
 				cerr << "Iter: " << iter << ", derr: " << cur_err - prev_err << endl;
 			#endif
 			++iter;
 		}
 		Vector3f end = getEndEffector();
 		#if DEBUG == 1
	 		cerr << "Error: " << cur_err << endl;
	 		cerr << "End: " << end[0] << " " << end[1] << "  " << end[2] << endl;
	 		cerr << "Goal: " << goal[0] << " " << goal[1] << " " << goal[2] << endl;
	 	#endif
 	}

 	Matrix<T, 1, 3> compute_joint_jacobian(int joint_num, Vector3f goal, Vector3f angle) {
 		T alpha = 1e-5;

 		Vector3f all_end = getEndEffector();
 		AngleAxis<T> t = AngleAxis<T>(alpha, angle);
 		joints[joint_num]->transform(t);

 		Vector3f new_all_end = getEndEffector();
 		joints[joint_num]->transform(t.inverse());

 		Vector3f diff = new_all_end - all_end;
 		Matrix<T, 1, 3> ret;
 		ret /= alpha;
 		ret << diff[0], diff[1], diff[2];
 		return ret;
 	}

 	void set_joints(vector<unique_ptr<BallJoint>> &&v) {
 		joints = move(v);
 	}

 	vector<unique_ptr<BallJoint>> joints;
 	Vector3f base;
};

#endif