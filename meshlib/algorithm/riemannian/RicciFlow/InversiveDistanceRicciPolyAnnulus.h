/*! \file InversiveDistanceRicciPolyAnnulus.h
 * \brief Euclidean Inversive Distance Ricci flow for Poly Annulus
 *  \author David Gu
 *  \date   documented on 11/15/2010
 *
 *	Algorithm for Euclidean Inversive Distance Ricci flow for Poly Annulus
 */

#ifndef _INVERSIVE_DISTANCE_RICCI_FLOW_POLY_ANNULUS_H_
#define _INVERSIVE_DISTANCE_RICCI_FLOW_POLY_ANNULUS_H_

#include <map>
#include <vector>

#include "InversiveDistanceRicciFlow.h"

namespace MeshLib
{
namespace RicciFlow
{
	/*! \brief CInversiveDistanceRicciFlowPolyAnnulus class
	 *  
	 *  Algorithm for Euclidean Inversive Distance Ricci flow for Poly Annulus
	 */
	 
 template<typename M>
 class CInversiveDistanceRicciFlowPolyAnnulus : public CInversiveDistanceRicciFlow<M>
  {
  public:
	  /*! \brief CInversiveDistanceRicciFlowPolyAnnulus constructor
	   *
	   *  call base class constructor 
	   */
	  CInversiveDistanceRicciFlowPolyAnnulus( M * pMesh );

	  /*!
	   *	Compute the desired metric
	   */
	  void _calculate_metric();

  protected:

	/*!
	 *	Set the target curvature on each vertex
	 */
    void    _set_target_curvature();
	/*!
	 *	optimize by the Newton's method
	 */
	 virtual void   _Newton( double threshold, double step_length );
  };

template<typename M>
CInversiveDistanceRicciFlowPolyAnnulus<M>::CInversiveDistanceRicciFlowPolyAnnulus( M * pMesh ): CInversiveDistanceRicciFlow( pMesh)
{
}



//set target curvature

template<typename M>
void CInversiveDistanceRicciFlowPolyAnnulus<M>::_set_target_curvature()
{
  for( M::MeshVertexIterator viter( m_pMesh ); !viter.end(); viter ++ )
  {
	  M::CVertex * v = *viter;
    v->target_k() = 0;
  }


  std::vector<M::CLoop*> & loops = m_boundary.loops();

  for( size_t i = 0; i < loops.size(); i ++ )
  {
	  if( i < 2 ) continue;
	  M::CLoop * pL = loops[i];
		
	  std::vector<M::CHalfEdge*> hes;

	  for( std::list<M::CHalfEdge*>::iterator hiter = pL->halfedges().begin();
		  hiter != pL->halfedges().end(); hiter ++ )
	  {
		  M::CHalfEdge * pH = *hiter;
		  hes.push_back( pH );
	  }
		
	  double length = 0;
	  for( size_t i = 0; i < hes.size(); i ++ )
	  {
		  M::CHalfEdge * pH = hes[i];
		  M::CEdge * e = m_pMesh->halfedgeEdge( pH );
		  length += e->length();
	  }
	
	  for( size_t i = 0; i < hes.size(); i ++ )
	  {
		  M::CHalfEdge * pH = hes[i];
		  M::CHalfEdge * nH = hes[(i+1)%hes.size()];
		  M::CVertex * pV = m_pMesh->halfedgeTarget( pH );

		  M::CEdge * pE = m_pMesh->halfedgeEdge( pH );
		  M::CEdge * nE = m_pMesh->halfedgeEdge( nH );

		  pV->target_k() = -2 * PI * ( pE->length() + nE->length() )/( 2 * length );
	  }


  }
};

//Newton's method for optimizing entropy energy

template<typename M>
void CInversiveDistanceRicciFlowPolyAnnulus<M>::_Newton( double threshold, double step_length )
{
	int num = m_pMesh->numVertices();
	double* b = new double[num];
	assert( b );
	memset(b,0,sizeof(double)*num);

	double* x = new double[num];
	assert( x != NULL );
	memset(x,0,sizeof(double)*num);


  	while( true )
	{
		//the order of the following functions really matters
		
 		_calculate_edge_length();
		_set_target_curvature();
		_calculate_corner_angle();
		_calculate_vertex_curvature();
		_calculate_edge_weight();

		double error =  _calculate_curvature_error();
		printf("Current error is %f\r\n", error );
		if( error < threshold) break;

		for( M::MeshVertexIterator viter( m_pMesh ); !viter.end(); viter ++  )
		{
			M::CVertex * v = *viter;
			v->huv()[0] = v->u();	
			v->u() = v->target_k() - v->k();
		}

		CPoisson<M> P(m_pMesh );
		P.solve2();
	

		for( M::MeshVertexIterator viter( m_pMesh ); !viter.end(); viter ++  )
		{
			M::CVertex * v = *viter;
			v->u() = v->huv()[0] + v->u() * step_length;
		}
		

  }
};


//compute metric

template<typename M>
void CInversiveDistanceRicciFlowPolyAnnulus<M>::_calculate_metric()
{

  //double error = 1e-6;
  double error = 5e-6;
  double step_length = 0.75;
  _Newton( error, step_length );

};

} //namespace RicciFlow
} //namespace MeshLib

#endif  