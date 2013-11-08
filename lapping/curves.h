#ifndef _CURVES_H
#define _CURVES_H

typedef double POINTTYPE;

typedef struct {
    POINTTYPE x;
    POINTTYPE y;
    POINTTYPE z;

    void set( int X, int Y, int Z){
        x=X; y=Y; z=Z;
    }
}Point;

class BSpline{
    private:
        int _n;        // no of control points are n+1
        int _m;        // no of knots are m+1
        int _d;

    public:

        double *kv;   // knot vector
        Point *cp; //control points

        BSpline(){
            _m = _n = _d = 0;
            cp = NULL;
            kv = NULL;
        }

        ~BSpline(){
            _m = _n = _d = 0;
            if( cp != NULL){
                delete [] cp;
                cp = NULL;
            }
            if( kv != NULL){
                delete [] kv;
                kv = NULL;
            }
        }

        bool initCurve( int controlPoints, int p){
            if( 1<= p && p<= controlPoints){
                _d = p;
                _n = controlPoints;
                cp = new Point[_n+1];
                _m = _n + _d + 1;
                kv = new double[_m+1];
                int span = _m - 2*_d - 1;
                for( int i=0; i<=_d; i++){
                    kv[i] = 0;
                    kv[_m-i] = 1;
                }
                for( int i=1; i<=span; i++)
                    kv[i+_d] = (double)(i)/(double)(span+1);
                return true;
            }
            return false;
        }

        int getD(){
            return _d;
        }

        int getN(){
            return _n;
        }

        int getM(){
            return _m;
        }

        double N( int i, int p, double u){
            if( 0<=i && 0<=p&&p<=_d && 0.0<=u&&u<=1.0 ){
                if( p==0){
                    if( kv[i] <= u&&u < kv[i+1] )
                        return 1;
                    return 0;
                }
                else if( kv[i+p]==kv[i] && kv[i+p+1]==kv[i+1] )
                    return 0;
                else if( kv[i+p]==kv[i])
                    return N(i+1,p-1,u)*(kv[i+p+1]-u)/(kv[i+p+1]-kv[i+1]);
                else if( kv[i+p+1]==kv[i+1] )
                    return N(i,p-1,u)*(u-kv[i])/(kv[i+p]-kv[i]);
                return N(i,p-1,u)*(u-kv[i])/(kv[i+p]-kv[i])+N(i+1,p-1,u)*(kv[i+p+1]-u)/(kv[i+p+1]-kv[i+1]);
            }
            return 0.0;
        }

        Point C( double u){
            Point r = { 0,0,0};
            if( 0.0<=u&&u<=1.0){
                for( int i=0; i<=_n; i++){
                    double nip = N(i,_d,u);
                    r.x += nip*cp[i].x;
                    r.y += nip*cp[i].y;
                    r.z += nip*cp[i].z;
                }
            }
            return r;
        }

        Point derivative( double u){
            Point r = { 0,0,0};
            if( 0.0<=u&&u<=1.0){
                for( int i=0; i<_n; i++){
                    double nip = N(i+1,_d-1,u);
                    double m = 1;
                    m = (double)(_d)/(double)(kv[i+_d+1]-kv[i+1]);
                    r.x += nip*m*(cp[i+1].x-cp[i].x);
                    r.y += nip*m*(cp[i+1].y-cp[i].y);
                    r.z += nip*m*(cp[i+1].z-cp[i].z);
                }
            }
            return r;
        }

        void lsApprox( int h, Point *data){ //h+1 data points
            double *t = new double[h+1];
            for( int i=0; i<=h; i++)
                t[i] = (double)(i)/(double)(h);
            cp[ 0] = data[0];
            cp[_n] = data[h];
            Point *Qk = new Point[h+1];
            for( int k=1; k<h; k++){
                double m1 = N(  0, _d, t[k]);
                double m2 = N( _n, _d, t[k]);
                Qk[k].x = data[k].x-m1*data[0].x-m2*data[h].x;
                Qk[k].y = data[k].y-m1*data[0].y-m2*data[h].y;
                Qk[k].z = data[k].z-m1*data[0].z-m2*data[h].z;
            }
            double *Q = new double[(_n-1)*3]; //3-D
            for( int i=1; i<_n; i++){
                double sx=0, sy=0, sz=0;
                for( int k=1; k<h; k++){
                    double m=N( i, _d, t[k]);
                    sx += m*Qk[k].x;
                    sy += m*Qk[k].y;
                    sz += m*Qk[k].z;
                }
                Q[3*(i-1)+0] = sx;
                Q[3*(i-1)+1] = sy;
                Q[3*(i-1)+2] = sz;
            }
            double *aN = new double[(h-1)*(_n-1)];
            for( int k=1,idx=0; k<h; k++)
                for( int i=1; i<_n; i++,idx++)
                    aN[idx] = N( i, _d, t[k]);
            double *NTN = new double[(_n-1)*(_n-1)];
            for( int j=0,idx=0; j<_n-1; j++)
                for( int i=0; i<_n-1; i++,idx++){
                    double s=0;
                    for( int k=0; k<h-1; k++)
                        s += aN[k*(_n-1)+j]*aN[k*(_n-1)+i];
                    NTN[idx] = s;
                }
            double *N_1 = new double[(_n-1)*(_n-1)];
            for( int i=0,k=0; i<_n-1; i++)
                for( int j=0; j<_n-1; j++,k++)
                    if( i==j)
                        N_1[k] = 1;
                    else
                        N_1[k] = 0;
            for( int i=0; i<_n-1; i++){
                double d = NTN[i*(_n-1)+i];
                for( int j=0; j<_n-1; j++){
                    if( i==j)
                        continue;
                    double r = NTN[j*(_n-1)+i] / d;
                    for( int k=0,lj=j*(_n-1),li=i*(_n-1); k<_n-1; k++,li++,lj++){
                        NTN[lj]-= r*NTN[li];
                        if((NTN[lj]>0&&NTN[lj]<1.0e-14)||(NTN[lj]<0&&NTN[lj]>-1.0e-14))
                            NTN[lj]=0;
                        N_1[lj]-= r*N_1[li];
                        if((N_1[lj]>0&&N_1[lj]<1.0e-14)||(N_1[lj]<0&&N_1[lj]>-1.0e-14))
                            N_1[lj]=0;
                    }
                }
            }
            for( int i=0; i<_n-1; i++){
                double d = NTN[i*(_n-1)+i];
                for( int k=0,li=i*(_n-1); k<_n-1; k++,li++)
                    N_1[li]/=d;
            }
            for( int i=0; i<_n-1; i++){
                double sx=0, sy=0, sz=0;
                for( int j=0; j<_n-1; j++){
                    sx += N_1[i*(_n-1)+j]*Q[j*3+0];
                    sy += N_1[i*(_n-1)+j]*Q[j*3+1];
                    sz += N_1[i*(_n-1)+j]*Q[j*3+2];
                }
                cp[i+1].set( sx, sy, sz);
            }
            delete [] N_1;
            delete [] NTN;
            delete [] aN;
            delete [] Q;
            delete [] Qk;
            delete [] t;
        }
};

#endif

/*
Basis func is saved as
N0,p
*/
