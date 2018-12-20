program ifdm_input

  integer :: i,j,t

  integer*4 :: nbytes
  integer*4 :: nt,nx,ny
  integer*4 :: t0_year, t0_month, t0_day, t0_hour, t0_min, t0_sec, dt
  real*4    :: xul, yul, dx, dy, xc, yc
  integer*4 :: epsg, missing
  character*4 :: pol, aggr
  character*10 :: ipol_class, conf
  character*30 :: author, email
  
  real*4, allocatable :: conc(:,:)

  character(len=256) :: filename

  if ( command_argument_count() .ne. 1 ) then
    write(*,*)'Usage:'
    write(*,*)' ifdm_read <rio_file.bin>'
    stop
  endif

  call get_command_argument(1,filename)
  write(*,*)'Opening : ', trim(filename)

  open(unit=1, file=trim(filename), access='stream', action='read', status='old', form='unformatted')
  read(unit=1) nbytes
  read(unit=1) nt, nx, ny, xul, yul, dx, dy, epsg, t0_year, t0_month, t0_day, t0_hour, t0_min, t0_sec, dt, missing
  read(unit=1) pol, aggr, ipol_class, conf, author, email


  ! allocate array for concentrations
  allocate( conc(ny,nx) ) ! number of rows( = ny) x number of columns : nx
  
  write(*,*)'*********************'
  write(*,*)'* HEADER INFORMAION *'
  write(*,*)'*********************'
  
  write(*,*)'bytes in header : ', nbytes
  write(*,*)'nt              : ', nt
  write(*,*)'nx              : ', nx ! number of columns
  write(*,*)'ny              : ', ny ! number of rows
  write(*,*)'xul             : ', xul
  write(*,*)'yul             : ', yul
  write(*,*)'dx              : ', dx
  write(*,*)'dy              : ', dy
  write(*,'(a,I4,a,I2,a,I2,a,I2,a,I2,a,I2)')'start date      : ', t0_year, '-', t0_month, '-', t0_day, &
    ' ', t0_hour, ':', t0_min, ':', t0_sec
  write(*,*)'dt              : ', dt
  write(*,*)'crs (epsg)      : ', epsg
  write(*,*)'missing_value   : ', missing
  write(*,*)'pollutant       : ', trim(pol)
  write(*,*)'aggregation     : ', trim(aggr)
  write(*,*)'interpolator    : ', trim(ipol_class)
  write(*,*)'configuration   : ', trim(conf)
  write(*,*)'author          : ', trim(author)
  write(*,*)'email           : ', trim(email)

  ! write ascii files for each concentration map...
  write(*,*)'*****************'
  write(*,*)'* DATA CONTENTS *'
  write(*,*)'*****************'
  do t=1,nt

   write(*,'(A,I4)') '- TIMESTEP ', t
   read(unit=1) conc
   do i=1,ny ! loop over rows
    do j=1,nx ! loop over columns
       write(*,'(F10.2)', advance="no") conc(i,j)
      ! reconstruct x,y
      !xc = xull+(j-0.5)*dx
      !yc = yull-(i-0.5)*dy
      

      ! te checken of dit corretct is...
      !write(*,'(F10.2,F10.2,F10.2)')xc,yc,conc(i,j)

    enddo
    write(*,*)
  enddo


enddo

end program ifdm_input
