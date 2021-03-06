#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const double TINY_NEAR_ZERO = 1.0E-12;

void MultiplyMat(register double *m1, register double *m2, register double *res, const int M1, const int N1, const int M2, const int N2);
int PseudoInverse(register double *inv, register double *matx, const int M, const int N);
int invert3x3(double *src, double *inv);

int _mossNRsvdcmp(register double **a, const int m, const int n, double w[], register double **v)
{
	int flag,i,its,j,jj,k,l,nm;
	double anorm,c,f,g,h,s,scale,x,y,z, rv11[n+1];
	
	register double *rv1 = rv11;
	g = scale = anorm = 0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(a[k][i]);
			if (scale) {
				for (k=i; k<=m; k++) {
					a[k][i] /= scale;
					s += a[k][i]*a[k][i];
				}
				f = a[i][i];
				g = -SIGN(sqrt(s), f);
				h=f*g-s;
				a[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
					f=s/h;
					for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
				}
				for (k=i;k<=m;k++) a[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(a[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					a[i][k] /= scale;
					s += a[i][k]*a[i][k];
				}
				f=a[i][l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				a[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
					for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) a[i][k] *= scale;
			}
		}
		anorm=FMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}

	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++) v[j][i]=(a[i][j]/a[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}

	for (i = IMIN(m,n); i>=1; i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) a[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
				f = (s/a[i][i])*g;
				for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
			for (j=i;j<=m;j++) a[j][i] *= g;
		} else for (j=i;j<=m;j++) a[j][i]=0.0;
		++a[i][i];
	}

	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ((double)(fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((double)(fabs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ((double)(fabs(f)+anorm) == anorm) break;
					g=w[i];
					h=PYTHAG(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=a[j][nm];
						z=a[j][i];
						a[j][nm]=y*c+z*s;
						a[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			if (its == 30) {
				//sprintf(err, "%s: no convergence in 30 svdcmp iterations", me);
				//biffAdd(MOSS, err);
				return 1;
			}
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=PYTHAG(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=PYTHAG(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=PYTHAG(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=a[jj][j];
					z=a[jj][i];
					a[jj][j]=y*c+z*s;
					a[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}

	//free(rv1);

	return 0;
}

int mossSVD(register double *U, register double *W, register double *V, register double *matx, const int M, const int N)
{
	register double **nrU, *nrW, **nrV;
	int problem, i;

	/* allocate arrays for the Numerical Recipes code to write into */
	nrU = (double **)malloc((M+1)*sizeof(double*));
	nrW = (double *)malloc((N+1)*sizeof(double));
	nrV = (double **)malloc((N+1)*sizeof(double*));
	problem = !(nrU && nrW && nrV);
	if (!problem) {
		problem = 0;
		for (i=1; i<=M; i++) {
			nrU[i] = (double *)malloc((N+1)*sizeof(double));
			problem |= !nrU[i];
		}
		for(i=1; i<=N; i++) {
			nrV[i] = (double *)malloc((N+1)*sizeof(double));
			problem |= !nrV[i];
		}
	}
	if (problem) {
		return 1;
	}

	/* copy from given matx into nrU */
	for (i=1; i<=M; i++) {
		memcpy(&(nrU[i][1]), matx + N*(i-1), N*sizeof(double));
	}

	/* HERE IT IS: do SVD */
	if (_mossNRsvdcmp(nrU, M, N, nrW, nrV)) {
		//sprintf(err, "%s: trouble in core SVD calculation", me);
		return 1;
	}

	/* copy results into caller's arrays */
	for (i=1; i<=M; i++) {
		memcpy(U + N*(i-1), &(nrU[i][1]), N*sizeof(double));
	}
	memcpy(W, &(nrW[1]), N*sizeof(double));
	for (i=1; i<=N; i++) {
		memcpy(V + N*(i-1), &(nrV[i][1]), N*sizeof(double));
	}

	/* free Numerical Recipes arrays */
	for (i=1; i<=M; i++) free(nrU[i]);

	free(nrU);
	free(nrW);

	for (i=1; i<=N; i++) free(nrV[i]);

	free(nrV);

	return 0;
}

int PseudoInverse(register double *inv, register double *matx, const int M, const int N) 
{
	register double *U, *W, *V, ans;
	int i, j, k;

	U = (double *)malloc(M*N*sizeof(double));
	W = (double *)malloc(N*sizeof(double));
	V = (double *)malloc(N*N*sizeof(double));

	if (!(U && W && V)) {
		return 1;
	}
	if (mossSVD(U, W, V, matx, M, N)) {
		//sprintf(err, "%s: trouble in SVD computation", me);
		return 1;
	}

	for (i=0; i<=N-1; i++) {
		if (fabs(W[i]) < TINY_NEAR_ZERO) 
		{
			return 1;
		}
	}

	for (i=0; i<=N-1; i++) {
		for (j=0; j<=M-1; j++) {
			ans = 0;
			for (k=0; k<=N-1; k++) {
				/* in V: row fixed at i, k goes through columns */
				/* in U^T: column fixed at j, k goes through rows ==>
				   in U: row fixed at j, k goes through columns */
				ans += V[k + N*i]*U[k + N*j]/W[k];
			}
			inv[j + M*i] = ans;
		}
	}

	free(U);
	free(W);
	free(V);
	
	return 0;
}

int invert3x3(double *src, double *inv)
{
	double m11 = src[0], m12 = src[1], m13 = src[2], m21 = src[3], m22 = src[4], m23 = src[5], m31 = src[6], m32 = src[7], m33 = src[8];
	double det = m11 * (m22*m33 - m23*m32) - m12 * (m21*m33 - m23*m31) + m13 * (m21*m32 - m22*m31);
	
	if(det == 0) return 1;
	
	det = 1.0 / det;
	
	inv[0] = det * (m22*m33 - m23*m32);
	inv[1] = det * (m13*m32 - m12*m33);
	inv[2] = det * (m12*m23 - m13*m22);
	
	inv[3] = det * (m23*m31 - m21*m33);
	inv[4] = det * (m11*m33 - m13*m31);
	inv[5] = det * (m13*m21 - m11*m23);
	
	inv[6] = det * (m21*m32 - m22*m31);
	inv[7] = det * (m12*m31 - m11*m32);
	inv[8] = det * (m11*m22 - m12*m21);
	
	return 0;
}

void MultiplyMat(register double *m1, register double *m2, register double *res, const int M1, const int N1, const int M2, const int N2) {
	int timesInner = IMIN( N1, M2 );
	int timesRows = M1;
	int timesCols = N2;
	double sum;

	int row, col, inner;
	for( row = 0; row < timesRows; row++ )
	{
		for( col = 0; col < timesCols; col++ )
		{
			sum = 0;
			for( inner = 0; inner < timesInner; inner++ )
			{
				sum += m1[row*N1 + inner] * m2[inner*N2 + col];
			}
			*(res++) = sum;
		}
	}
}
