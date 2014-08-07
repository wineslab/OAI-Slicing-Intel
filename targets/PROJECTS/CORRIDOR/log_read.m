clear all
close all

top_dir = 'P:\florian\CORRIDOR\trials2 train'; % needs to be updated according to your computer
d1 = dir(fullfile(top_dir,'UHF','*.log'));
d2 = dir(fullfile(top_dir,'2.6GHz','*.log'));





start_time = [1.400489088000000e+09 1.400493112000000e+09 1.400499696000000e+09 1.400506864000000e+09];

for idx=1:length(d1)
    data1{idx}=csvread(fullfile(top_dir,'UHF',d1(idx).name),1,0);
    data2{idx}=csvread(fullfile(top_dir,'2.6GHz',d2(idx).name),1,0);
    
    
    
    frame_start1(idx) = ceil(data1{idx}(find(data1{idx}(:,1)>start_time(idx),1,'first'),3)/92160000)*100;
    frame_start2(idx) = ceil(data2{idx}(find(data2{idx}(:,1)>start_time(idx),1,'first'),3)/368640000)*100;
    
    %% find the first dataset with valid GPS signal and throw away everything before
    idx1_start = find(data1{idx}(:,6)>0,1,'first');
    data1{idx}(1:idx1_start-1,:)=[];
    idx2_start = find(data2{idx}(:,6)>0,1,'first');
    data2{idx}(1:idx2_start-1,:)=[];
    
    rtime1 = data1{idx}(:,1) - data1{idx}(1,1);
    rtime2 = data2{idx}(:,1) - data2{idx}(1,1);
    
    %% plot gps coordinates
    figure(idx*10+1);
    hold off
    plot(data1{idx}(1:100:end,7),data1{idx}(1:100:end,8),'rx')
    hold on
    plot(data2{idx}(1:100:end,7),data2{idx}(1:100:end,8),'bx')
    xlabel('lat [deg]');
    ylabel('lon [deg]')
    legend('UHF','2.6GHz')
    title(sprintf('Run %d',idx));
    
    %% plot RSSI
    % TODO: convert time (in unix epoch) into something more meaninful
    figure(idx*10+2);
    hold off
    plot(rtime1,smooth(data1{idx}(:,13),100),'r')
    hold on
    plot(rtime2,smooth(data2{idx}(:,13),100),'b')
    plot(rtime2,smooth(data2{idx}(:,19),100),'c')
    plot(rtime2,smooth(data2{idx}(:,25),100),'m')
    legend('UHF','2.6GHz card 1','2.6GHz card 2','2.6GHz card 3');
    xlabel('time [seconds]')
    ylabel('RSSI [dBm]')
    title(sprintf('Run %d',idx));
    
    
    
    %% measured distance (km)
    % We can get the distance between the base station and the RX antenna from the GPS coordinates
    distances1=zeros(size(data1{idx},1),1);
    distances2=zeros(size(data2{idx},1),1);
    for i=1:size(data1{idx},1)
        distances1(i)=Dist_Calc_from_GPS(data1{idx}(i,7),data1{idx}(i,8),48.25073056,1.55481944);
    end
    
    for i=1:size(data2{idx},1)
        distances2(i)=Dist_Calc_from_GPS(data2{idx}(i,7),data2{idx}(i,8),48.25073056,1.55481944);
    end
    
    
    figure (100+idx)
    subplot(1,2,1)
    plot(rtime1,distances1,'r',rtime1,smooth(data1{idx}(:,13),100),'b')
    title(sprintf('Run %d with the measured distance : UHF',idx));
    xlabel('time [s]');
    legend('distance [km]','RSSI [dBm]');
    
    subplot(1,2,2)
    plot(rtime2,distances2,'r',rtime2,smooth(data2{idx}(:,13),100),'b')
    title(sprintf('Run %d with the measured distance : 2.6GHz ',idx));
    xlabel('time [s]');
    legend('distance [km]','RSSI [dBm]');
    %% estimated distance under the assumption of a constant speed
    % We assume that the TGV speed is constant and then we find the distances to the base station with the time vector
    
    TGV_speed=82.5;%constant TGV speed in m/s
    
    
    
    [RSSI_max1,I_RSSI_max1]=max(data1{idx}(:,13));%we find the index corresponding to the maximum of RSSI
    time01=rtime1(I_RSSI_max1)*ones(length(rtime1),1);%time corresponding to the maximim of RSSI
    new_distances1=(TGV_speed*abs(rtime1-time01))/1000+min(distances1)*ones(length(rtime1),1);% new distance in km. is is minimum when the RSSI is maximum
    
    
    [RSSI_max2,I_RSSI_max2]=max(data2{idx}(:,13));
    time02=rtime2(I_RSSI_max2)*ones(length(rtime2),1);
    new_distances2=(TGV_speed*abs(rtime2-time02))/1000+min(distances2)*ones(length(rtime2),1);% distance in km
    
    
    if (idx==2)%For Run 2, there is an anomalous peak for the RSSI at the end. Here we ignore it
        [RSSI_max2,I_RSSI_max2]=max(data2{idx}(1:32900,13));
        time02=rtime2(I_RSSI_max2)*ones(length(rtime2),1);
        new_distances2=(TGV_speed*abs(rtime2-time02))/1000+min(distances2)*ones(length(rtime2),1);% distance in km
    end
    
    figure (200+idx)
    subplot(1,2,1)
    plot(rtime1,new_distances1,'r',rtime1,smooth(data1{idx}(:,13),100),'b')
    title(sprintf('Run %d with the estimated distance : UHF',idx));
    xlabel('time [s]');
    legend('distance [km]','RSSI [dBm]');
    
    subplot(1,2,2)
    plot(rtime2,new_distances2,'r',rtime2,smooth(data2{idx}(:,13),100),'b')
    title(sprintf('Run %d with the estimated distance : 2.6GHz ',idx));
    xlabel('time [s]');
    legend('distance [km]','RSSI [dBm]');
    
    
    
    
    
    
    
    
    
    
    
    %% rssi(dBm) versus distance (log scale)
    % We will plot the rssi versus the distance with the data before the passing of the train, and with the data after the passingof the train
    
    % we heuristically determine a starting point and a ending point for the linear fitting
    if idx==1
        
        distance_before_break1_start=0.4;%in km
        distance_before_break1_end=7.5;
        distance_before_break2_start=0.4;
        distance_before_break2_end=7.5;
        
        distance_after_break1_start=0.9;%in km
        distance_after_break1_end=4.5;
        distance_after_break2_start=0.9;
        distance_after_break2_end=4.5;
    end
    
    if idx==2
        distance_before_break1_start=0.8;%in km
        distance_before_break1_end=4.5;
        distance_before_break2_start=0.8;
        distance_before_break2_end=4.5;
        
        distance_after_break1_start=0.5;%in km
        distance_after_break1_end=7.5;
        distance_after_break2_start=0.5;
        distance_after_break2_end=7.5;
    end
    
    if idx==3
        distance_before_break1_start=5;%in km
        distance_before_break1_end=23;
        distance_before_break2_start=5;
        distance_before_break2_end=23;
        
        distance_after_break1_start=2.274;%in km
        distance_after_break1_end=5.376;
        distance_after_break2_start=5.996;
        distance_after_break2_end=7.613;
    end
    
    if idx==4
        distance_before_break1_start=0.1176;%in km
        distance_before_break1_end=3.489;
        distance_before_break2_start=0.1344;
        distance_before_break2_end=3.78;
        
        distance_after_break1_start=0.5;%in km
        distance_after_break1_end=8;
        distance_after_break2_start=0.5;
        distance_after_break2_end=8;
    end
    
    % indexes of the starting and ending points with the data before the passing of the
    % train
    index_break1_before_start=1;
    index_break2_before_start=1;
    index_break1_before_end=1;
    index_break2_before_end=1;
    
    % indexes of the starting and ending points with the data after the passing of the
    % train
    index_break1_after_start=I_RSSI_max1;
    index_break2_after_start=I_RSSI_max2;
    index_break1_after_end=I_RSSI_max1;
    index_break2_after_end=I_RSSI_max2;
    
    %starting points
    while (index_break1_before_start<length(new_distances1)) && (new_distances1(index_break1_before_start)>distance_before_break1_start)
        index_break1_before_start=index_break1_before_start+1;
    end
    while (index_break2_before_start<length(new_distances2)) && (new_distances2(index_break2_before_start)>distance_before_break2_start)
        index_break2_before_start=index_break2_before_start+1;
    end
    %ending points
    while (index_break1_before_end<length(new_distances1)) && (new_distances1(index_break1_before_end)>distance_before_break1_end)
        index_break1_before_end=index_break1_before_end+1;
    end
    while (index_break2_before_end<length(new_distances2)) && (new_distances2(index_break2_before_end)>distance_before_break2_end)
        index_break2_before_end=index_break2_before_end+1;
    end
    
    
    %starting points
    while (index_break1_after_start<length(new_distances1)) && (new_distances1(index_break1_after_start)<distance_after_break1_start)
        index_break1_after_start=index_break1_after_start+1;
    end
    while (index_break2_after_start<length(new_distances2)) && (new_distances2(index_break2_after_start)<distance_after_break2_start)
        index_break2_after_start=index_break2_after_start+1;
    end
    
    %ending points
    while (index_break1_after_end<length(new_distances1)) && (new_distances1(index_break1_after_end)<distance_after_break1_end)
        index_break1_after_end=index_break1_after_end+1;
    end
    while (index_break2_after_end<length(new_distances2)) && (new_distances2(index_break2_after_end)<distance_after_break2_end)
        index_break2_after_end=index_break2_after_end+1;
    end
    
    
    
    
    
    figure(idx*10+3)
    
    
    subplot(2,1,1)
    
    hold off
    linearCoef1_before = polyfit(10*log10(new_distances1(index_break1_before_end:index_break1_before_start)),data1{idx}(index_break1_before_end:index_break1_before_start,13),1);
    linearFit1_before = polyval(linearCoef1_before,10*log10(new_distances1(index_break1_before_end:index_break1_before_start)));
    semilogx(new_distances1(1:I_RSSI_max1),data1{idx}(1:I_RSSI_max1,13),'rx',new_distances1(index_break1_before_end:index_break1_before_start),linearFit1_before,'r-')
    display(sprintf('Run %d :slope UHF before: %f',idx,linearCoef1_before(1)))
    
    
    hold on
    linearCoef2_before = polyfit(10*log10(new_distances2(index_break2_before_end:index_break2_before_start)),data2{idx}(index_break2_before_end:index_break2_before_start,13),1);
    linearFit2_before = polyval(linearCoef2_before,10*log10(new_distances2(index_break2_before_end:index_break2_before_start)));
    semilogx(new_distances2(1:I_RSSI_max2),data2{idx}(1:I_RSSI_max2,13),'bx',new_distances2(index_break2_before_end:index_break2_before_start),linearFit2_before,'b-')
    display(sprintf('Run %d :slope 2.6GHz before: %f',idx,linearCoef2_before(1)))
    
    
    title(sprintf('Run %d: With the data before the passing of the train',idx))
    legend('UHF','UHF:linear fitting','2.6GHz card 1','2.6GHz card 1:linear fitting');
    xlabel('distance [km]')
    ylabel('RSSI [dBm]')
    
    subplot(2,1,2)
    
    hold off
    linearCoef1_after = polyfit(10*log10(new_distances1(index_break1_after_start:index_break1_after_end)),data1{idx}(index_break1_after_start:index_break1_after_end,13),1);
    linearFit1_after = polyval(linearCoef1_after,10*log10(new_distances1(index_break1_after_start:index_break1_after_end)));
    semilogx(new_distances1(I_RSSI_max1:end),data1{idx}(I_RSSI_max1:end,13),'rx',new_distances1(index_break1_after_start:index_break1_after_end),linearFit1_after,'r-')
    display(sprintf('Run %d :slope UHF after: %f',idx,linearCoef1_after(1)))
    
    
    hold on
    linearCoef2_after = polyfit(10*log10(new_distances2(index_break2_after_start:index_break2_after_end)),data2{idx}(index_break2_after_start:index_break2_after_end,13),1);
    linearFit2_after = polyval(linearCoef2_after,10*log10(new_distances2(index_break2_after_start:index_break2_after_end)));
    semilogx(new_distances2(I_RSSI_max2:end),data2{idx}(I_RSSI_max2:end,13),'bx',new_distances2(index_break2_after_start:index_break2_after_end),linearFit2_after,'b-')
    display(sprintf('Run %d :slope 2.6GHz after: %f',idx,linearCoef2_after(1)))
    
    title(sprintf('Run %d: With the data after the passing of the train',idx))
    legend('UHF','UHF:linear fitting','2.6GHz card 1','2.6GHz card 1:linear fitting');
    xlabel('distance [km]')
    ylabel('RSSI [dBm]')
    
    
    % Zoom on the linear fitting
    
    figure(idx*10+4)
    
    
    subplot(2,1,1)
    
    hold off
    
    linearCoef1_before = polyfit(10*log10(new_distances1(index_break1_before_end:index_break1_before_start)),data1{idx}(index_break1_before_end:index_break1_before_start,13),1);
    linearFit1_before = polyval(linearCoef1_before,10*log10(new_distances1(index_break1_before_end:index_break1_before_start)));
    semilogx(new_distances1(index_break1_before_end:index_break1_before_start),data1{idx}(index_break1_before_end:index_break1_before_start,13),'rx',new_distances1(index_break1_before_end:index_break1_before_start),linearFit1_before,'r-')
    %display(sprintf('Run %d :slope UHF before: %f',idx,linearCoef1_before(1)))
    
    hold on
    
    linearCoef2_before = polyfit(10*log10(new_distances2(index_break2_before_end:index_break2_before_start)),data2{idx}(index_break2_before_end:index_break2_before_start,13),1);
    linearFit2_before = polyval(linearCoef2_before,10*log10(new_distances2(index_break2_before_end:index_break2_before_start)));
    semilogx(new_distances2(index_break2_before_end:index_break2_before_start),data2{idx}(index_break2_before_end:index_break2_before_start,13),'bx',new_distances2(index_break2_before_end:index_break2_before_start),linearFit2_before,'b-')
    %display(sprintf('Run %d :slope 2.6GHz before: %f',idx,linearCoef2_before(1)))
    
    title(sprintf('Run %d: With the data before the passing of the train',idx))
    legend('UHF','UHF:linear fitting','2.6GHz card 1','2.6GHz card 1:linear fitting');
    xlabel('distance [km]')
    ylabel('RSSI [dBm]')
    
    subplot(2,1,2)
    
    hold off
    
    
    linearCoef1_after = polyfit(10*log10(new_distances1(index_break1_after_start:index_break1_after_end)),data1{idx}(index_break1_after_start:index_break1_after_end,13),1);
    linearFit1_after = polyval(linearCoef1_after,10*log10(new_distances1(index_break1_after_start:index_break1_after_end)));
    semilogx(new_distances1(index_break1_after_start:index_break1_after_end),data1{idx}(index_break1_after_start:index_break1_after_end,13),'rx',new_distances1(index_break1_after_start:index_break1_after_end),linearFit1_after,'r-')
    %display(sprintf('Run %d :slope UHF after: %f',idx,linearCoef1_after(1)))
    hold on
    
    linearCoef2_after = polyfit(10*log10(new_distances2(index_break2_after_start:index_break2_after_end)),data2{idx}(index_break2_after_start:index_break2_after_end,13),1);
    linearFit2_after = polyval(linearCoef2_after,10*log10(new_distances2(index_break2_after_start:index_break2_after_end)));
    semilogx(new_distances2(index_break2_after_start:index_break2_after_end),data2{idx}(index_break2_after_start:index_break2_after_end,13),'bx',new_distances2(index_break2_after_start:index_break2_after_end),linearFit2_after,'b-')
    %display(sprintf('Run %d :slope 2.6GHz after: %f',idx,linearCoef2_after(1)))
    title(sprintf('Run %d: With the data after the passing of the train',idx))
    legend('UHF','UHF:linear fitting','2.6GHz card 1','2.6GHz card 1:linear fitting');
    xlabel('distance [km]')
    ylabel('RSSI [dBm]')
    
    
    
    
    
    
    
    
    
end