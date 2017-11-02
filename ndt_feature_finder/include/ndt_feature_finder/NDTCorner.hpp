#ifndef DAS_NDTCORNER_10072016
#define DAS_NDTCORNER_10072016

#include <math.h>
#include <pcl/point_types.h>
#include "ndt_map/ndt_map.h"

#include "Utils.hpp"

namespace perception_oru{
	
	namespace ndt_feature_finder{
		
		class NDTCornerBundle{
		protected:
			
			struct EigenValVec{
				Eigen::Vector3d eigenvec;
				double eigenval;
				Eigen::Vector3d NormEigen(){
					return eigenvec * eigenval;
				}
			};
			
			bool _mean_set;
			
			std::vector<boost::shared_ptr< lslgeneric::NDTCell > > _cell1;
			std::vector<boost::shared_ptr< lslgeneric::NDTCell > > _cell2;
			double _angle;
			double _direction;
			
			Eigen::Matrix3d _eigen_vector;
			Eigen::Vector3d _eigen_values;
			Eigen::Vector3d _mean;
			
		public:
			NDTCornerBundle(): _mean_set(false){}
			NDTCornerBundle(const Eigen::Vector3d& m) :
				_mean(m), _mean_set(true)
			{}
			void push_back_cell1(const boost::shared_ptr< lslgeneric::NDTCell >& c1){_cell1.push_back(c1);}
			void push_back_cell2(const boost::shared_ptr< lslgeneric::NDTCell >& c2){_cell2.push_back(c2);}
			void setMean(const Eigen::Vector3d& m){_mean_set = true; _mean = m;}
			void setAngle(double a){_angle = a;}
			void setDirection(double d){_direction = d;}
			
			const Eigen::Vector3d& getMean() const {return _mean;}
			Eigen::Vector3d getMean(){return _mean;}
			const Eigen::Matrix3d& getEigenVectors(){return _eigen_vector;}
			const Eigen::Vector3d& getEigenValues(){return _eigen_values;}
			
			void inverseDistanceWeighting(){
				
			}
			
			void gaussian(){
				assert(_mean_set == true);
				
				auto cell1 = _cell1[0];
				auto cell2 = _cell2[0];
				
				EigenValVec biggest_cell1, smallest_cell1, biggest_cell2, smallest_cell2;
				
				getEigenVectors(*cell1, biggest_cell1, smallest_cell1);
				getEigenVectors(*cell2, biggest_cell2, smallest_cell2);
				
				Eigen::Vector3d side_point1_cell1;
				Eigen::Vector3d side_point2_cell1;
				side_point1_cell1 = cell1->getMean() + smallest_cell1.NormEigen();
				side_point2_cell1 = cell1->getMean() - smallest_cell1.NormEigen();
				
				Eigen::Vector3d side_point1_cell2;
				Eigen::Vector3d side_point2_cell2;
				side_point1_cell2 = cell2->getMean() + smallest_cell2.NormEigen();
				side_point2_cell2 = cell2->getMean() - smallest_cell2.NormEigen();
				
// 				std::cout << "All points " << side_point1_cell1 << " \n\n " << side_point1_cell2 << " \n\n " << side_point2_cell2 << std::endl <<  smallest_cell2.NormEigen() << std::endl;

				//First collision line
				auto collision = collisionRay(biggest_cell1.eigenvec, side_point1_cell1, biggest_cell2.eigenvec, side_point1_cell2);
				
// 				std::cout << "Collision " << collision << " nesxtr " << biggest_cell1.eigenvec << " nesxtr " << side_point1_cell1 << " nesxtr " << biggest_cell2.eigenvec << " nesxtr " << side_point1_cell2 << " " << std::endl;
				//Second collision line with only one of the two line moved. It doesn't matter which but it needs to be only one.
				auto collision1 = collisionRay(biggest_cell1.eigenvec, side_point1_cell1, biggest_cell2.eigenvec, side_point2_cell2);
// 				std::cout << "Collision2 " << collision1 << " nesxtr " << biggest_cell1.eigenvec << " nesxtr " << side_point1_cell1 << " nesxtr " << biggest_cell2.eigenvec << " nesxtr " << side_point2_cell2 << " " << std::endl;
				
// 				std::cout << "Mean " << _mean << std::endl;
// 				std::cout << "RES1 \n " << collision - _mean << std::endl;
// 				std::cout << "RES2\n " << collision1 - _mean << std::endl;
				
				Eigen::Vector3d v1(0,0,0);
				_eigen_vector << collision - _mean , collision1 - _mean, v1 ;
				_eigen_values << 1, 1, 1;
				
// 				std::cout << "FINAl\n " << _eigen_vector << std::endl;
				
			}
			
		private:
			
			void getEigenVectors(const lslgeneric::NDTCell& cell, EigenValVec& biggest, EigenValVec& smallest) const {
				
				Eigen::Vector3d eigenval;
				Eigen::Matrix3d eigenvec;
				getEigenVectors2D(cell, eigenval, eigenvec);
			// 	std::cout << "Eigen sorted" << std::endl;
				if(eigenval(1) > eigenval(0)){
					biggest.eigenvec = eigenvec.col(1);
					smallest.eigenvec = eigenvec.col(0);
					biggest.eigenval = eigenval(1);
					smallest.eigenval = eigenval(0);
				}
				else{
					biggest.eigenvec = eigenvec.col(0);
					smallest.eigenvec = eigenvec.col(1);
					biggest.eigenval = eigenval(0);
					smallest.eigenval = eigenval(1);
				}
			}
			
		};
		
		
		class NDTCorner{
			
		protected:
			
			
			double _x_cell_size, _y_cell_size, _z_cell_size;
			//CHange to NDTCornerBundle
			std::vector< boost::shared_ptr< lslgeneric::NDTCell > > _corners;
			std::vector< Eigen::Vector3d > _corners_position;
			std::vector< cv::Point2d > _opencv_corners;
			std::vector< cv::Point2d > _opencv_corners_position;
			/**
			 * Pair of angle size between edges + direction of each corner
			 */
			std::vector<std::pair<double,double> > _angles;
			int _neighbor_size;
			
		public:
			NDTCorner() : _neighbor_size(2){};
			
			/**
			 * @brief Return the orientation of the cell in deg compared to [SOMETHING]
			 */
// 			double getCellOrientation(const lslgeneric::NDTCell& cell) const ;
			
			/**
			 * @brief return true if the cell correspond to a corner
			 */
			bool cellIsCorner(const lslgeneric::NDTMap& map, const lslgeneric::NDTCell& cell, const std::vector< boost::shared_ptr< lslgeneric::NDTCell > >& allCells, NDTCornerBundle& corner) ;
			
			/**
			 * @brief calculate and return a vector of all corners in ndt_map
			 */
			std::vector< boost::shared_ptr< lslgeneric::NDTCell > > getAllCorners(const lslgeneric::NDTMap& map) ;
			
			/**
			 * @brief return all the cells around cell that possess a gaussian and are maximum the resolution of the map away.
			 */
			std::vector< boost::shared_ptr< lslgeneric::NDTCell > > getClosestCells(const lslgeneric::NDTMap& map, const lslgeneric::NDTCell& cell, int neig_size) const ;
			
			/**
			 * @brief return the angle between two Vectors.
			 */
// 			double getAngle(const Eigen::Vector3d& vec, const Eigen::Vector3d& vec2) const;
			
			/**
			 * @brief return the index of the biggest eigen vector in the 2D plane xy
			 */
// 			int getBiggestEigenVector2D(const lslgeneric::NDTCell& cell, Eigen::Vector3d& eigenval, Eigen::Matrix3d& eigenvec) const;
			
			/**
			 * @brief return the angle width and direction of all corner detected by it
			 */
			
			
			
			
			void clear(){
				_x_cell_size = _y_cell_size = _z_cell_size = 0;
				_corners.clear();
				_corners_position.clear();
				_opencv_corners.clear();
				_opencv_corners_position.clear();
				_angles.clear();
			}
			
			size_t size() const {return _corners.size();}
			std::vector< boost::shared_ptr< lslgeneric::NDTCell > >& getCorners(){return _corners;}
			const std::vector< boost::shared_ptr< lslgeneric::NDTCell > >& getCorners() const {return _corners;}
			std::vector< cv::Point2d >& getCvCorners(){return _opencv_corners;}
			const std::vector< cv::Point2d >& getCvCorners() const {return _opencv_corners;}
			std::vector< Eigen::Vector3d >& getAccurateCorners(){return _corners_position;}
			const std::vector< Eigen::Vector3d >& getAccurateCorners() const {return _corners_position;}
			std::vector< cv::Point2d >& getAccurateCvCorners(){return _opencv_corners_position;}
			const std::vector< cv::Point2d >& getAccurateCvCorners() const {return _opencv_corners_position;}
			const std::vector<std::pair<double,double> >& getAngles() const {assert(_angles.size() ==_opencv_corners_position.size()); return _angles;}
			std::vector<std::pair<double,double> >& getAngles(){assert(_angles.size() ==_opencv_corners_position.size()); return _angles;}
			
			
			void exportCorners(std::ostream& out){
				for(size_t i = 0 ; i < _opencv_corners.size() ; ++i){
					std::cout << _opencv_corners[i].x << " " << _opencv_corners[i].y << std::endl;
					out << _opencv_corners[i].x << " " << _opencv_corners[i].y << std::endl;
				}
			}
			void exportAccurateCorners(std::ostream& out){
				for(size_t i = 0 ; i < _opencv_corners_position.size() ; ++i){
					std::cout << _opencv_corners_position[i].x << " " << _opencv_corners_position[i].y << std::endl;
					out << _opencv_corners_position[i].x << " " << _opencv_corners_position[i].y << std::endl;
				}
			}
			
			void readCorners(std::ifstream& in){
				while(true){
					cv::Point2d p;
// 					std::cout << "f"<<std::endl;
// 					if(in.eof()) break;
					in >> p.x;
// 					std::cout << "s : "<< p.x << std::endl;
// 					if(in.eof()) break;
					in >> p.y;
// 					std::cout << "Pushing " << p << std::endl;
					if(in.eof()) break;
					_opencv_corners.push_back(p);
				}
			}
			
			void readAccurateCorners(std::ifstream& in){
				while(true){
					cv::Point2d p;
// 					std::cout << "f"<<std::endl;
// 					if(in.eof()) break;
					in >> p.x;
// 					std::cout << "s : "<< p.x << std::endl;
// 					if(in.eof()) break;
					in >> p.y;
// 					std::cout << "Pushing " << p << std::endl;
					if(in.eof()) break;
					_opencv_corners_position.push_back(p);
				}
			}
			
		private:
			
			
			bool gotAngledNDT(const lslgeneric::NDTMap& map, std::vector< boost::shared_ptr< lslgeneric::NDTCell > >& neighbor, NDTCornerBundle& corner) ;
			/**
			 * @brief remove doubles of corners. ATTENTION : it does not update the openCV corners
			 */
			void clearAccurateCorner();
			/**
			 * @brief transform ndt_cell mean and eigen point to cv::Point2d
			 */
			void toOpenCV();
			/**
			 * @brief calculate the angles of each corners
			 */
			void calculateAngles(const lslgeneric::NDTMap& map);
			
			std::vector< lslgeneric::NDTCell* > getCellsPointingToward(std::vector< lslgeneric::NDTCell* >& neighbor, const lslgeneric::NDTCell& cell) const;
			bool AlignedNDT(const std::vector< lslgeneric::NDTCell* >& neighbor, const lslgeneric::NDTCell& cell);
			
		};
		
	}
}



#endif