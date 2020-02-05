<?xml version="1.0" encoding="utf-8"?>
<o:OfficeConfig xmlns:o="urn:schemas-microsoft-com:office:office">
 <o:services o:GenerationTime="2018-10-19T08:01:36">
  <!--Build: 16.0.11016.36852-->
  <o:default>
   <o:ticket o:headerName="Authorization" o:headerValue="{}" />
  </o:default>
  <o:service o:name="Research">
   <o:url>https://rr.office.microsoft.com/research/query.asmx</o:url>
  </o:service>
  <o:service o:name="ORedir">
   <o:url>https://o15.officeredir.microsoft.com/r</o:url>
  </o:service>
  <o:service o:name="ORedirSSL">
   <o:url>https://o15.officeredir.microsoft.com/r</o:url>
  </o:service>
  <o:service o:name="ClViewClientHelpId">
   <o:url>https://[MAX.BaseHost]/client/results</o:url>
  </o:service>
  <o:service o:name="ClViewClientHome">
   <o:url>https://[MAX.BaseHost]/client/results</o:url>
  </o:service>
  <o:service o:name="ClViewClientTemplate">
   <o:url>http://ocsa.office.microsoft.com/client/15/help/template</o:url>
  </o:service>
  <o:service o:name="ClViewClientSearch">
   <o:url>https://[MAX.BaseHost]/client/results</o:url>
  </o:service>
  <o:service o:name="ClViewClientSearchRedir">
   <o:url>https://support.office.microsoft.com/client/results</o:url>
  </o:service>
  <o:service o:name="ClViewClientUpdate">
   <o:url>http://ocsa.office.microsoft.com/client/15/help/clvupd</o:url>
  </o:service>
  <o:service o:name="HelpContactSupport" o:authentication="1">
   <o:url>https://[MAX.BaseHost]/home/chat</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[MAX.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[MAX.ResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MruDocuments" o:authentication="2">
   <o:url>https://ocws.[OSI.BaseHost]/ocs/docs/recent</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MruPlaces" o:authentication="2">
   <o:url>https://ocws.[OSI.BaseHost]/ocs/locations/recent</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MruV2Service" o:authentication="2">
   <o:url>https://ocws.[OSI.BaseHost]/ocs/v2/recent</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MruDocumentsSharedWithMe" o:authentication="2">
   <o:url>https://ocws.[OSI.BaseHost]/ocs/docs/sharedwithme</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MruDocumentsSharedWithMeV2" o:authentication="2">
   <o:url>https://ocws.[OSI.BaseHost]/ocs/docs/v2.0/sharedwithme</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneDriveLiveUrl">
   <o:url>https://onedrive.live.com</o:url>
  </o:service>
  <o:service o:name="OneDriveLogUploadService" o:authentication="2">
   <o:url>https://storage.live.com/clientlogs/uploadlocation</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://onedrive.live.com" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ODCInsertMedia" o:authentication="1">
   <o:url>https://insertmedia.bing.office.net/odc/insertmedia</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="InsertMediaTelemetry">
   <o:url>https://hubblecontent.osi.office.net/contentsvc/api/telemetry</o:url>
  </o:service>
  <o:service o:name="OfficeOnlineContent" o:authentication="1">
   <o:url>https://insertmedia.bing.office.net/images/officeonlinecontent/browse?cp=Bing</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="OfficeOnlineContentClipArt">
   <o:url>https://insertmedia.bing.office.net/images/officeonlinecontent/browse?cp=ClipArt</o:url>
  </o:service>
  <o:service o:name="OfficeOnlineContentFb" o:authentication="1">
   <o:url>https://insertmedia.bing.office.net/images/officeonlinecontent/browse?cp=Facebook</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="OfficeOnlineContentFl" o:authentication="1">
   <o:url>https://insertmedia.bing.office.net/images/officeonlinecontent/browse?cp=Flickr</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="OfficeOnlineContentIcons">
   <o:url>https://hubblecontent.osi.office.net/contentsvc/microsofticon?</o:url>
  </o:service>
  <o:service o:name="OfficeOnlineContentImages" o:authentication="1">
   <o:url>https://hubblecontent.osi.office.net/contentsvc/browse?</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="OfficeOnlineContentOD" o:authentication="1">
   <o:url>https://insertmedia.bing.office.net/images/officeonlinecontent/browse?cp=OneDrive</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="OfficeOnlineContentOEmbed">
   <o:url>https://hubble.[OSI.BaseHost]/mediasvc/api/media/oembed</o:url>
  </o:service>
  <o:service o:name="OfficeOnlineContent3DModel" o:authentication="1">
   <o:url>https://hubblecontent.osi.office.net/contentsvc/browse?cp=remix3d</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="officeapps.live.com" />
  </o:service>
  <o:service o:name="OfficeOnlineContentVideos">
   <o:url>https://insertmedia.[OSI.ONETBaseHost]/insertmedia</o:url>
  </o:service>
  <o:service o:name="HubbleLoggingSvc">
   <o:url>https://hubble.[OSI.BaseHost]/mediasvc/api/media/log</o:url>
  </o:service>
  <o:service o:name="BingInsertMediaImmersive">
   <o:url>https://insertmedia.bing.office.net/images/hosted?host=office&amp;adlt=strict&amp;hostType=ImmersiveApp</o:url>
  </o:service>
  <o:service o:name="HomeRealmDiscovery">
   <o:url>https://odc.[OSI.BaseHost]/odc/v2.0/hrd</o:url>
  </o:service>
  <o:service o:name="GetFederationProvider">
   <o:url>https://odc.[OSI.BaseHost]/odc/emailhrd/getfederationprovider</o:url>
  </o:service>
  <o:service o:name="GetIdentityProvider">
   <o:url>https://odc.[OSI.BaseHost]/odc/emailhrd/getidp</o:url>
  </o:service>
  <o:service o:name="ODCServicesCatalog">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/catalog</o:url>
  </o:service>
  <o:service o:name="ODCSites" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/v{0}/sites</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ODCUserConnectedServices" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/userconnected</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ODCClientDeveloperHelp">
   <o:url>https://odc.[OSI.BaseHost]/odc/help/clientdeveloper</o:url>
  </o:service>
  <o:service o:name="ODCManageServiceRedir" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/manageserviceredir.aspx</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerName="X-Office-Auth-ODCSM" />
  </o:service>
  <o:service o:name="ODCSMAddRepairService" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/liveredir</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerName="X-Office-Auth-ODCSM" />
  </o:service>
  <o:service o:name="ODCSMManageService" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/liveredir</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:parameterName="t" />
  </o:service>
  <o:service o:name="ODCSMServiceAdded" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/serviceadd</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ODCSMServiceDetails" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/servicedetails</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ODCSMServiceRemoved" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/serviceremove</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ODCSMReportServiceError" o:authentication="2">
   <o:url>https://odc.[OSI.BaseHost]/odc/servicemanager/reportserviceerror</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="DSIRoamingSettings" o:authentication="2">
   <o:url>https://roaming.[OSI.BaseHost]/rs/RoamingSoapService.svc</o:url>
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:idprovider="3" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OfficeLicensingService15" o:authentication="2">
   <o:url>https://ols.[OSI.BaseHost]/olsc/OlsClient.svc/OlsClient</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OfficeLicensingServiceGlobal">
   <o:url>https://ols.[OSI.BaseHost]/olsc/OlsClient.svc/OlsClient</o:url>
  </o:service>
  <o:service o:name="LicensingRestHost">
   <o:url>https://ols.[OSI.BaseHost]/</o:url>
  </o:service>
  <o:service o:name="OfficeHomeStoreCatalogEndpoint">
   <o:url>https://displaycatalog.mp.microsoft.com/v7.0/products/CFQ7TTC0K5DM</o:url>
  </o:service>
  <o:service o:name="OfficeSoloStoreCatalogEndpoint">
   <o:url>https://displaycatalog.mp.microsoft.com/v7.0/products/CFQ7TTC0K5BC</o:url>
  </o:service>
  <o:service o:name="RevereActivities" o:authentication="2">
   <o:url>https://revere.[OSI.ONETBaseHost]/api/v{0}/documents/{1}</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="RMSOnline" o:authentication="2">
   <o:url>-</o:url>
   <o:ticket o:policy="MCMBI" o:idprovider="2" o:target="[RMSOnline.BaseHost]" />
   <o:ticket o:idprovider="3" o:resourceId="https://api.aadrm.com/" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="BroadcastService" o:authentication="1">
   <o:url>https://broadcast.[OSI.BaseHost]/m/broadcasthost.asmx</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="AccessNounDefinitionsB2">
   <o:url>https://support.content.office.microsoft.com/en-us/static/AF102833395.xml</o:url>
  </o:service>
  <o:service o:name="AccessNounDefinitionsRTM">
   <o:url>https://support.content.office.microsoft.com/en-us/static/AF102833403.xml</o:url>
  </o:service>
  <o:service o:name="LiveOAuthLoginBase">
   <o:url>https://login.[Live.WebHost]</o:url>
  </o:service>
  <o:service o:name="LiveOAuthLoginEnd">
   <o:url>https://login.[Live.WebHost]/oauth20_desktop.srf</o:url>
  </o:service>
  <o:service o:name="LiveOAuthLoginError">
   <o:url>https://login.[Live.WebHost]/err.srf</o:url>
  </o:service>
  <o:service o:name="LiveOAuthSignOut">
   <o:url>https://login.[Live.WebHost]/logout.srf</o:url>
  </o:service>
  <o:service o:name="LiveOAuthGetToken">
   <o:url>https://login.[Live.WebHost]/oauth20_token.srf</o:url>
  </o:service>
  <o:service o:name="OneDriveNotificationService" o:authentication="1">
   <o:url>https://skyapi.live.net/Activity/{0}</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerValue="WLID1.1 {}" />
  </o:service>
  <o:service o:name="OneDriveBusinessNotificationService" o:authentication="1">
   <o:url>https://api.onedrive.com</o:url>
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneDrivePublicApi" o:authentication="2">
   <o:url>https://[OneDriveApi.BaseHost]/v1.0/drive/root</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneDrivePublicApiRoot" o:authentication="2">
   <o:url>https://[OneDriveApi.BaseHost]/v1.0</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI" o:idprovider="2" o:target="[OSI.RootHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="SkyDriveSkyDocs" o:authentication="2">
   <o:url>https://[Live.DocumentAPIHost]/SkyDocsService.svc</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
  </o:service>
  <o:service o:name="SkyDriveSharing" o:authentication="2">
   <o:url>https://[Live.DocumentAPIHost]/SharingService.svc</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="SkyDriveSyncClientUpsell">
   <o:url>https://cdn.odc.officeapps.live.com/odc/xml?resource=OneDriveSyncClientUpsell</o:url>
  </o:service>
  <o:service o:name="LiveProfileService" o:authentication="2">
   <o:url>https://directory.services.[Live.WebHost]/profile/Profile.asmx</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="LiveProfileServicesGetInfo" o:authentication="2">
   <o:url>https://pf.directory.live.com/profile/mine/WLX.Profiles.IC.json</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerName="PS-MSAAuthTicket" />
  </o:service>
  <o:service o:name="LiveProfileServicesGetPhoneNumber" o:authentication="2">
   <o:url>https://pf.directory.live.com/profile/mine/System.ShortCircuitProfile.json</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="people.directory.[Live.WebHost]" o:headerName="PS-MSAAuthTicket" />
  </o:service>
  <o:service o:name="LiveCreateProfile" o:authentication="1">
   <o:url>https://profile.[Live.WebHost]/cid-%s/</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="PreinstalledAppsQuery" o:authentication="1">
   <o:url>https://[OMEX.BaseHost]/appinstall/preinstalled</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="LiveEditProfile" o:authentication="1">
   <o:url>https://profile.[Live.WebHost]/home</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="LiveContacts" o:authentication="1">
   <o:url>https://contacts.[MSN.WebHost]/ABService/ABService.asmx</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" />
  </o:service>
  <o:service o:name="OCSettingsUrl" o:authentication="2">
   <o:url>https://ocps.manage.microsoft.com</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://ocps.manage.microsoft.com" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="SkyDriveSignUpUpsell">
   <o:url>https://cdn.odc.officeapps.live.com/odc/xml?resource=OneDriveSignUpUpsell</o:url>
  </o:service>
  <o:service o:name="SkyDriveSignUpUpsellImage">
   <o:url>https://cdn.odc.officeapps.live.com/odc/stat/images/OneDriveUpsell.png</o:url>
  </o:service>
  <o:service o:name="OutlookConnectorManifest">
   <o:url>https://support.content.office.microsoft.com/en-us/static/AF102819889.xml</o:url>
  </o:service>
  <o:service o:name="LiveIdFederatedToOrgIdForDesktop">
   <o:url>-</o:url>
   <o:ticket o:policy="MBI_FED_SSL_C14N" o:idprovider="1" o:target="[Live.FederatedAuthHost]" />
  </o:service>
  <o:service o:name="LiveIdFederatedToOrgIdForImmersive">
   <o:url>-</o:url>
   <o:ticket o:policy="LBI_FED_STS_CLEAR" o:idprovider="1" o:target="[Live.FederatedAuthHost]" />
  </o:service>
  <o:service o:name="OrgIdSigninRedir">
   <o:url>https://office.microsoft.com/en-us/signinredir.aspx?url=https%3a%2f%2foffice.microsoft.com%2f&amp;hurl=7EAE03D69861684965F1762C11D3663A89CB2605&amp;ipt=2</o:url>
  </o:service>
  <o:service o:name="NLGEnglishAssistance">
   <o:url>https://ssl.bing.com/dict/?view=officemoe&amp;ulang=zh-cn&amp;tlang=en-us</o:url>
  </o:service>
  <o:service o:name="NLGEnglishAssistanceIcon">
   <o:url>https://ssl.bing.com/dict/img/BingDict_E2C.PNG</o:url>
  </o:service>
  <o:service o:name="CISMobileAssociation">
   <o:url>https://partnerservices.getmicrosoftkey.com/PartnerProvisioning.svc/v1/subscriptions</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="partnerservices.getmicrosoftkey.com" />
  </o:service>
  <o:service o:name="CISRedemptionService">
   <o:url>https://pbsub.microsoft.com/RedemptionService/v1/redeem</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="redemptionservices.getmicrosoftkey.com" />
  </o:service>
  <o:service o:name="Nexus">
   <o:url>https://nexus.[OSI.BaseHost]</o:url>
  </o:service>
  <o:service o:name="NexusRules">
   <o:url>https://nexusrules.[OSI.BaseHost]</o:url>
  </o:service>
  <o:service o:name="LiveRestAPI">
   <o:url>https://apis.live.net/v5.0/</o:url>
  </o:service>
  <o:service o:name="FontService">
   <o:url>https://fs.microsoft.com/fs/1.3</o:url>
  </o:service>
  <o:service o:name="FontService2">
   <o:url>https://fs.microsoft.com/fs/2.2</o:url>
  </o:service>
  <o:service o:name="ResourceServiceEndpoint2">
   <o:url>https://fs.microsoft.com/fs/4.5</o:url>
  </o:service>
  <o:service o:name="ResourceServiceiOS2">
   <o:url>https://fs.microsoft.com/fs/iOS/2.8</o:url>
  </o:service>
  <o:service o:name="ResourcesOnDemandEndpoint">
   <o:url>https://officecdn.microsoft.com/pr/uiraas/ios/</o:url>
  </o:service>
  <o:service o:name="ProofingDownloadServiceEndpoint">
   <o:url>https://fs.microsoft.com/fs/2.6</o:url>
  </o:service>
  <o:service o:name="OArtResourceServiceEndpoint">
   <o:url>https://fs.microsoft.com/fs/oart/1.5</o:url>
  </o:service>
  <o:service o:name="OArtModel3DResourceServiceEndpoint">
   <o:url>https://fs.microsoft.com/fs/oart/model3d/res/1.0</o:url>
  </o:service>
  <o:service o:name="ExcelRichValueIcons">
   <o:url>https://fs.microsoft.com/fs/RichValueIcons/3.0</o:url>
  </o:service>
  <o:service o:name="OutlookFlighting">
   <o:url>http://olkflt.edog.officeapps.live.com/olkflt/outlookflighting.svc/api/glides</o:url>
  </o:service>
  <o:service o:name="OutlookWeatherMSN">
   <o:url>http://weather.service.msn.com/data.aspx</o:url>
  </o:service>
  <o:service o:name="SSExcelCS">
   <o:url>https://excelcs.[OSI.BaseHost]/xlauto/excelautomation.svc/XlAutomation</o:url>
  </o:service>
  <o:service o:name="SSExcelCSREST">
   <o:url>https://excelcs.[OSI.BaseHost]/xlauto/excelautomation.svc/rest</o:url>
  </o:service>
  <o:service o:name="SSExcelPS">
   <o:url>https://excelps.[OSI.BaseHost]/exlps/excelprint.svc/exlPrint</o:url>
  </o:service>
  <o:service o:name="SSPPTCS">
   <o:url>https://pptcs.[OSI.BaseHost]/pptauto/PowerpointAutomation.svc/PptAutomation</o:url>
  </o:service>
  <o:service o:name="SSPPTCSREST">
   <o:url>https://pptcs.[OSI.BaseHost]/pptauto/PowerpointAutomation.svc/rest</o:url>
  </o:service>
  <o:service o:name="SSPPTPS">
   <o:url>https://pptps.[OSI.BaseHost]/pptps/powerpointprint.svc/PptPrint</o:url>
  </o:service>
  <o:service o:name="SSPPTSS">
   <o:url>https://pptss.[OSI.BaseHost]/pptss/powerpointsample.svc/PptSample</o:url>
  </o:service>
  <o:service o:name="SSPPTSGS">
   <o:url>https://pptsgs.[OSI.BaseHost]/pptsgs/PowerpointSuggestion.svc/PptSuggestion</o:url>
  </o:service>
  <o:service o:name="StreamAPI">
   <o:url>https://api.microsoftstream.com/api/</o:url>
  </o:service>
  <o:service o:name="StreamVideoBase">
   <o:url>https://web.microsoftstream.com/video/</o:url>
  </o:service>
  <o:service o:name="PPTQuickStarterBase">
   <o:url>https://pptcts.[OSI.BaseHost]/pptcts/</o:url>
  </o:service>
  <o:service o:name="PPTQuickStarterPageBase">
   <o:url>https://pptcts.[OSI.BaseHost]/pptcts/Home.aspx</o:url>
  </o:service>
  <o:service o:name="PPTQuickStarterThumbnailBase">
   <o:url>https://officecdn.microsoft.com/pr/pptcts/thumbnail_2x/thumbnail</o:url>
  </o:service>
  <o:service o:name="SSWordCS">
   <o:url>https://wordcs.[OSI.BaseHost]/wordauto/wordautomation.svc/wordautomation</o:url>
  </o:service>
  <o:service o:name="SSWordCSREST">
   <o:url>https://wordcs.[OSI.BaseHost]/wordauto/wordautomation.svc/rest</o:url>
  </o:service>
  <o:service o:name="SSWordPS">
   <o:url>https://wordps.[OSI.BaseHost]/wrdps/wordprint.svc/wrdprint</o:url>
  </o:service>
  <o:service o:name="PowerBI" o:authentication="2">
   <o:url>https://api.powerbi.com/v1.0/myorg/imports</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALPBIResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="PowerBIGroupsApi" o:authentication="2">
   <o:url>https://api.powerbi.com/v1.0/myorg/groups</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALPBIResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="AzureMachineLearningEndpointServiceUrl">
   <o:url>https://management.azure.com/</o:url>
  </o:service>
  <o:service o:name="BingGeospatialEndpointServiceUrl">
   <o:url>https://dev.virtualearth.net/REST/V1/GeospatialEndpoint/</o:url>
  </o:service>
  <o:service o:name="BingSpellerServiceUrl">
   <o:url>https://www.bing.com/api/v4/spelling/proof</o:url>
  </o:service>
  <o:service o:name="NLProofingServiceUrl">
   <o:url>https://nleditor.[OSI.ONETBaseHost]/NlEditor/CloudSuggest/V1</o:url>
  </o:service>
  <o:service o:name="PolicyTipsSyncService" o:authentication="2">
   <o:url>https://dataservice.protection.outlook.com/PolicySync/PolicySync.svc/SyncFile</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://dataservice.o365filtering.com" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="DLPGraphApi" o:authentication="2">
   <o:url>https://graph.windows.net/</o:url>
   <o:ticket o:idprovider="3" o:resourceId="https://graph.windows.net" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="DLPGraphApiPPE" o:authentication="2">
   <o:url>https://graph.ppe.windows.net/</o:url>
   <o:ticket o:idprovider="3" o:resourceId="https://graph.ppe.windows.net" o:authorityUrl="[ADALPPEAuthorityUrl]" />
  </o:service>
  <o:service o:name="PolicyTipsSyncServicePPE" o:authentication="2">
   <o:url>https://dataservice.o365filtering.com/PolicySync/PolicySync.svc/SyncFile</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://dataservice.o365filtering.com" o:authorityUrl="[ADALPPEAuthorityUrl]" />
  </o:service>
  <o:service o:name="InsightsDesktop">
   <o:url>https://uci.officeapps.live.com/OfficeInsights/web/views/insights.desktop.html</o:url>
  </o:service>
  <o:service o:name="InsightsImmersive">
   <o:url>https://uci.officeapps.live.com/OfficeInsights/web/views/insights.immersive.html</o:url>
  </o:service>
  <o:service o:name="InsightsService" o:authentication="2">
   <o:url>http://insights.microsoft.com:8799</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="api://insights.microsoft.com:8799/918dae65-7fb5-4a7f-928a-0161d75d2db8" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="DSCRedemptionService">
   <o:url>https://res.getmicrosoftkey.com/api/redemptionevents</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="https://rpsticket.partnerservices.getmicrosoftkey.com" />
  </o:service>
  <o:service o:name="ACSTokenRedemption">
   <o:url>https://posarprodcssservice.accesscontrol.windows.net/v2/OAuth2-13</o:url>
  </o:service>
  <o:service o:name="IdentityService">
   <o:url>https://identity.[OSI.ONETBaseHost]/v1/token</o:url>
  </o:service>
  <o:service o:name="DSCRedemptionConfig">
   <o:url>https://ols.[OSI.BaseHost]/olsc/olsconfig.svc/redemption/locales</o:url>
  </o:service>
  <o:service o:name="OfficeEntityBaseUrl">
   <o:url>https://entity.[OSI.ONETBaseHost]/</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="https://[OSI.ONETBaseHost]" />
   <o:ticket o:idprovider="3" o:resourceId="https://entity.[OSI.ONETBaseHost]/" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OfficeEntityResearchUrl">
   <o:url>https://entity.[OSI.ONETBaseHost]/</o:url>
  </o:service>
  <o:service o:name="OfficeEntityMSATarget">
   <o:url>https://[OSI.ONETBaseHost]</o:url>
  </o:service>
  <o:service o:name="OfficeEntity">
   <o:url>https://cdn.entity.[OSI.ONETBaseHost]/OfficeEntity/web/views/juno.desktop.cshtml</o:url>
  </o:service>
  <o:service o:name="OfficeEntityMac">
   <o:url>https://cdn.entity.[OSI.ONETBaseHost]/OfficeEntity/web/views/juno.mac.cshtml</o:url>
  </o:service>
  <o:service o:name="RedemptionFlightingBucket">
   <o:url>https://ols.[OSI.BaseHost]/olsc/olsconfig.svc/redemption/flighting/</o:url>
  </o:service>
  <o:service o:name="MyAccount">
   <o:url>https://go.microsoft.com/fwlink/?LinkID=808147</o:url>
  </o:service>
  <o:service o:name="WhatsNewContent">
   <o:url>https://contentstorage.[OSI.ONETBaseHost]/whatsnew</o:url>
  </o:service>
  <o:service o:name="OfficeHubSubscription" o:authentication="2">
   <o:url>https://[OMEX.BaseCoSubHost]/myaccount/api/account.svc/officehub</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="OfficeHubCarouselSubscription" o:authentication="2">
   <o:url>https://[OMEX.BaseCoSubHost]/myaccount/api/account.svc/subscription</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="OfficeHubCarouselCdn">
   <o:url>https://contentstorage.[OSI.ONETBaseHost]/getofficecarouselcore/index.html</o:url>
  </o:service>
  <o:service o:name="MsGraphBase">
   <o:url>https://[MsGraphBaseURL]/</o:url>
  </o:service>
  <o:service o:name="MsGraphGetCalendarview" o:authentication="2">
   <o:url>https://[MsGraphBaseURL]/[MsGraphVersion]/me/calendarview/</o:url>
   <o:ticket o:idprovider="3" o:resourceId="https://[MsGraphBaseURL]/" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MsGraphGetOutlookMeetingAttachments" o:authentication="2">
   <o:url>https://[MsGraphBaseURL]/[MsGraphVersion]/me/events/{0}/attachments</o:url>
   <o:ticket o:idprovider="3" o:resourceId="https://[MsGraphBaseURL]/" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MsGraphGetPhoto" o:authentication="2">
   <o:url>https://[MsGraphBaseURL]/[MsGraphVersion]/me/photo/%24value</o:url>
   <o:ticket o:idprovider="3" o:resourceId="https://[MsGraphBaseURL]/" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="GraphImportUrl">
   <o:url>https://shredder.[OSI.ONETBaseHost]/ShredderService/web/desktop/views/main.cshtml</o:url>
  </o:service>
  <o:service o:name="FioIntelligenceBaseUrl">
   <o:url>https://fileiointelligence.osi.officeppe.net/</o:url>
  </o:service>
  <o:service o:name="EnrichmentUrl">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/web/view/desktop/main.cshtml</o:url>
  </o:service>
  <o:service o:name="EnrichmentDisambiguateUrl" o:authentication="1">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/Resolve/v1</o:url>
   <o:ticket o:idprovider="3" o:headerName="Authorization" o:headerValue="Bearer {}" o:resourceId="https://enrichment.[OSI.ONETBaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="https://[OSI.ONETBaseHost]" o:headerName="Authorization" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
  </o:service>
  <o:service o:name="EnrichmentBloomFilterBaseUrl">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/web/Metadata/</o:url>
  </o:service>
  <o:service o:name="EnrichmentMetadataUrl">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/web/Metadata/metadata.json</o:url>
  </o:service>
  <o:service o:name="EnrichmentRefreshUrl" o:authentication="1">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/Refresh/v1</o:url>
   <o:ticket o:idprovider="3" o:headerName="Authorization" o:headerValue="Bearer {}" o:resourceId="https://enrichment.[OSI.ONETBaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="https://[OSI.ONETBaseHost]" o:headerName="Authorization" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
  </o:service>
  <o:service o:name="EnrichmentReportContentUrl">
   <o:url>https://petrol.office.microsoft.com/v1/feedback</o:url>
  </o:service>
  <o:service o:name="EnrichmentSearchUrl" o:authentication="1">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/Search/v1</o:url>
   <o:ticket o:idprovider="3" o:headerName="Authorization" o:headerValue="Bearer {}" o:resourceId="https://enrichment.[OSI.ONETBaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="https://[OSI.ONETBaseHost]" o:headerName="Authorization" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
  </o:service>
  <o:service o:name="EnrichmentHandshakeBaseUrl">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/v2.1601090631428</o:url>
  </o:service>
  <o:service o:name="EnrichmentIPCheckUrl">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/ipcheck/v1</o:url>
  </o:service>
  <o:service o:name="EnrichmentWACUrl">
   <o:url>https://enrichment.[OSI.ONETBaseHost]/OfficeEnrichment/web/view/web/main.cshtml</o:url>
  </o:service>
  <o:service o:name="UrlReputationService" o:authentication="2">
   <o:url>https://na01.oscs.protection.outlook.com/api/SafeLinksApi/GetPolicy</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://dataservice.o365filtering.com" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="AADGraphGetMe" o:authentication="2">
   <o:url>https://[AADGraphBaseURL]/me?api-version=1.6</o:url>
   <o:ticket o:idprovider="3" o:resourceId="https://[AADGraphBaseURL]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="PinValidation">
   <o:url>https://ols.[OSI.BaseHost]/olsc/olsconfig.svc/pin/v2/</o:url>
  </o:service>
  <o:service o:name="SfBAutoDiscover">
   <o:url>https://webdir.online.lync.com/autodiscover/autodiscoverservice.svc/root/</o:url>
  </o:service>
  <o:service o:name="ExchangeAutoDiscover">
   <o:url>https://autodiscover-s.outlook.com/autodiscover/autodiscover.xml</o:url>
  </o:service>
  <o:service o:name="ExchangeBase">
   <o:url>https://[EXO.BaseHost]/</o:url>
  </o:service>
  <o:service o:name="ExchangeWebService" o:authentication="2">
   <o:url>https://[EXO.BaseHost]/ews/exchange.asmx</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[EXO.BaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="FormsODataApi" o:authentication="2">
   <o:url>https://[Forms.BaseHost]/formapi/api/</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="c9a559d2-7aab-4f13-a6ed-e7e9c52aec87" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="FormsDesignPage">
   <o:url>https://[Forms.BaseHost]/Pages/DesignPage.aspx</o:url>
  </o:service>
  <o:service o:name="StreamWebResource" o:authentication="2">
   <o:url>https://stream.microsoft.com</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="cf53fce8-def6-4aeb-8d30-b158e7b1cf83" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="AugmentationLoopService">
   <o:url>https://augloop.office.com</o:url>
  </o:service>
  <o:service o:name="LinkRequestApiPageTitleRetrieval">
   <o:url>https://uci.[OSI.BaseHost]/OfficeInsights/Insights/v2</o:url>
  </o:service>
  <o:service o:name="PPTOasisFD">
   <o:url>https://pptsgs.[OSI.BaseHost]/pptsgs/FrontDoor.ashx</o:url>
  </o:service>
  <o:service o:name="WordOasisFD">
   <o:url>https://wordsgs.[OSI.BaseHost]/wordsgs/FrontDoor.ashx</o:url>
  </o:service>
  <o:service o:name="XlOasisFD">
   <o:url>https://excelsgs.[OSI.BaseHost]/xlfrontdoor/FrontDoor.ashx</o:url>
  </o:service>
  <o:service o:name="OMEXSSOConsentPage" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/client/consent.aspx</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="c606301c-f764-4e6b-aa45-7caaaea93c9a" o:authorityUrl="[ADALAuthorityUrl]" />
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="OMEXSideloadingAddinSSOConsentPage" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/client/consentsideloading.aspx</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="c606301c-f764-4e6b-aa45-7caaaea93c9a" o:authorityUrl="[ADALAuthorityUrl]" />
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="OMEXRemoveAppsAndSsoConsentOrgId" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/orgid/apps/remove</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="c606301c-f764-4e6b-aa45-7caaaea93c9a" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="IssueServiceSearchURL">
   <o:url>https://issues.office.microsoft.com/api/search</o:url>
  </o:service>
  <o:service o:name="IssueServiceMeTooURL">
   <o:url>https://issues.office.microsoft.com/api/confirmIssue</o:url>
  </o:service>
  <o:service o:name="SubstrateServiceAutoSuggestBaseUrl" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/search/api/v1/</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="DynamicCanvasContent">
   <o:url>https://contentstorage.[OSI.ONETBaseHost]/dynamiccanvas</o:url>
  </o:service>
  <o:service o:name="DynamicCanvasStaticContent">
   <o:url>https://contentstorage.[OSI.ONETBaseHost]/dynamiccanvas/static</o:url>
  </o:service>
  <o:service o:name="ProgressUIExpUrl">
   <o:url>https://contentstorage.osi.office.net/dynamiccanvas/progressui/index.html</o:url>
  </o:service>
  <o:service o:name="ProgressUICommercialExpUrl">
   <o:url>https://contentstorage.osi.office.net/dynamiccanvas/documentvirality/prod/index.html</o:url>
  </o:service>
  <o:service o:name="TellMeSuggestionsBaseUrl">
   <o:url>https://tellmeservice.[OSI.ONETBaseHost]/tellmeservice/api/suggestions</o:url>
  </o:service>
  <o:service o:name="OneDriveSharesApi" o:authentication="1">
   <o:url>https://[OneDriveApi.BaseHost]/v1.0/shares/</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="InfoProtectSyncService" o:authentication="2">
   <o:url>https://syncservice.protection.outlook.com/PolicySync/PolicySync.svc/SyncFile</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://dataservice.o365filtering.com/" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ConsumerMyAccountSubscription" o:authentication="2">
   <o:url>https://[OMEX.BaseCoSubHost]/myaccount/api/account.svc/subscription</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="SubstrateSignalService" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/api/beta/me/Signals</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Outlook.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Outlook.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="EditorServiceCheckApiUrl">
   <o:url>https://nleditor.[OSI.ONETBaseHost]/NlEditor/Check/V1</o:url>
  </o:service>
  <o:service o:name="EditorServiceConfigApiUrl">
   <o:url>https://nleditor.[OSI.ONETBaseHost]/NlEditor/Config/V2</o:url>
  </o:service>
  <o:service o:name="EditorServiceInstrumentationApiUrl">
   <o:url>https://nleditor.[OSI.ONETBaseHost]/NlEditor/Instrumentation/V1</o:url>
  </o:service>
  <o:service o:name="EditorServiceLanguageInfoApiUrl">
   <o:url>https://nleditor.[OSI.ONETBaseHost]/NlEditor/LanguageInfo/V1</o:url>
  </o:service>
  <o:service o:name="UAPActivityService" o:authentication="2">
   <o:url>https://outlook.office365.com/api/v1.0/me/Activities</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Outlook.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Outlook.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="SubstrateAutoDiscoverServiceBaseUrl">
   <o:url>https://outlook.office365.com/autodiscover/autodiscover.json</o:url>
  </o:service>
  <o:service o:name="SubstrateOfficeIntelligenceService" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/OfficeIntelligence/v1.0</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="BingSpeechConsumerEndpoint">
   <o:url>wss://speech.platform.bing.com/speech/recognition/dictation/office/v1</o:url>
  </o:service>
  <o:service o:name="BingSpeechCompliantEndpoint">
   <o:url>wss://officespeech.platform.bing.com/speech/recognition/dictation/office/v1</o:url>
  </o:service>
  <o:service o:name="PowerLiftFrontDeskBaseUrl">
   <o:url>https://powerlift-frontdesk.acompli.net</o:url>
  </o:service>
  <o:service o:name="PowerLiftGymBaseUrl">
   <o:url>https://powerlift.acompli.net</o:url>
  </o:service>
  <o:service o:name="SubstrateOfficeIntelligenceInsightsService" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/OfficeIntelligence/v1.0/insights</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="SubstrateOfficeIntelligenceIngestionService" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/OfficeIntelligence/v1.0/ingestion</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="SubstrateOfficeFeedDocuments" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/OfficeFeed/api/beta/documents</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="TeamsAriaCollector">
   <o:url>https://mobile.pipe.aria.microsoft.com/Collector/3.0</o:url>
  </o:service>
  <o:service o:name="TeamsAuthEndpoint" o:authentication="2">
   <o:url>https://[Teams.BaseHost]/api/authsvc/v1.0/authz</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Teams.BaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="TeamsECSEndpoint">
   <o:url>https://config.teams.microsoft.com</o:url>
  </o:service>
  <o:service o:name="TeamsFEEndpoint">
   <o:url>https://teams.microsoft.com</o:url>
  </o:service>
  <o:service o:name="TeamsSchedulerService" o:authentication="2">
   <o:url>https://scheduler.[Teams.BaseHost]/teams/v1/meetings</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Teams.BaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="TeamsMTEndpoint">
   <o:url>https://authsvc.teams.microsoft.com</o:url>
  </o:service>
  <o:service o:name="TeamsUpgradeV2">
   <o:url>https://[Teams.BaseHost]/api/mt/beta/me/settings/teamsupgradev2</o:url>
  </o:service>
  <o:service o:name="TeamsUserAggregateSettings">
   <o:url>https://[Teams.BaseHost]/api/mt/beta/users/useraggregatesettings</o:url>
  </o:service>
  <o:service o:name="FeedbackUploadUrl">
   <o:url>https://sas.office.microsoft.com/upload.ashx</o:url>
  </o:service>
  <o:service o:name="EcsExperimentRequestUrl">
   <o:url>https://config.edge.skype.com/config/v1/Office</o:url>
  </o:service>
  <o:service o:name="EdgeExperimentRequestUrl">
   <o:url>https://ocos-office365-s2s.msedge.net/ab</o:url>
  </o:service>
  <o:service o:name="TasExperimentRequestUrl">
   <o:url>https://client-office365-tas.msedge.net/ab</o:url>
  </o:service>
  <o:service o:name="TranslatorService">
   <o:url>https://ogma.osi.office.net/TradukoApi/api/v1.0/</o:url>
  </o:service>
  <o:service o:name="PlannerBaseUrl">
   <o:url>https://tasks.office.com</o:url>
  </o:service>
  <o:service o:name="ProjectGraphBaseUrl">
   <o:url>https://graph.microsoft.com</o:url>
  </o:service>
  <o:service o:name="ProjectLandingPage">
   <o:url>https://portfolio.microsoft.com</o:url>
  </o:service>
  <o:service o:name="XrmAuthorizationUri">
   <o:url>https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/authorize</o:url>
  </o:service>
  <o:service o:name="XrmGlobalDiscovery">
   <o:url>https://globaldisco.crm.dynamics.com</o:url>
  </o:service>
  <o:service o:name="AriaBrowserPipeUrl">
   <o:url>https://office.pipe.aria.microsoft.com/Collector/3.0/</o:url>
  </o:service>
  <o:service o:name="DSCRedemptionServiceURL">
   <o:url>https://officesetup.getmicrosoftkey.com</o:url>
  </o:service>
  <o:service o:name="SkydocsServiceUrl">
   <o:url>https://d.[Live.DocumentAPIHost]</o:url>
  </o:service>
  <o:service o:name="DSCRedemptionServiceEndpoint">
   <o:url>https://token.cp.microsoft.com/api/redemptions/?subscription-key=f61754ff1afb40b48fa24fbbc2b2b7ba</o:url>
  </o:service>
  <o:service o:name="PrivacyPaneServiceBaseUrl">
   <o:url>https://pptsgs.[OSI.BaseHost]/pptsgs/resources/</o:url>
  </o:service>
  <o:service o:name="AppsForOfficeCDNUrl">
   <o:url>https://appsforoffice.microsoft.com</o:url>
  </o:service>
  <o:service o:name="AppLauncherClientServiceUrliOS">
   <o:url>https://shell.suite.office.com:1443</o:url>
  </o:service>
  <o:service o:name="ExchangeAutoDiscoverV2Url">
   <o:url>https://outlook.office365.com/autodiscover/autodiscover.json</o:url>
  </o:service>
  <o:service o:name="BIVisualHostAgaveUrl">
   <o:url>https://ovisualuiapp.azurewebsites.net/pbiagave/</o:url>
  </o:service>
  <o:service o:name="ODSProdUrl">
   <o:url>https://api.diagnostics.office.com</o:url>
  </o:service>
  <o:service o:name="ODSPPEUrl">
   <o:url>https://o365diagnosticsppe-web.cloudapp.net</o:url>
  </o:service>
  <o:service o:name="ODSSDFUrl">
   <o:url>https://api.diagnosticssdf.office.com</o:url>
  </o:service>
  <o:service o:name="LiveIDCloudSettingsUrl">
   <o:url>https://settings.outlook.com</o:url>
  </o:service>
  <o:service o:name="IDConfigUrl">
   <o:url>https://login.microsoftonline.com/</o:url>
  </o:service>
  <o:service o:name="AutoDetectProdAPIUrl">
   <o:url>https://prod-global-autodetect.acompli.net/autodetect</o:url>
  </o:service>
  <o:service o:name="AutoDetectTestAPIUrl">
   <o:url>https://dev0-api.acompli.net/autodetect</o:url>
  </o:service>
  <o:service o:name="FlowBaseHostUrl">
   <o:url>https://[Flow.BaseHost]</o:url>
  </o:service>
  <o:service o:name="ImageToDocServiceEndpoint">
   <o:url>https://imagetodoc.[OSI.BaseHost]</o:url>
  </o:service>
  <o:service o:name="FlowEnvironmentsUrl" o:authentication="2">
   <o:url>https://api.[Flow.BaseHost]/providers/Microsoft.ProcessSimple/environments</o:url>
   <o:ticket o:idprovider="3" o:headerName="Authorization" o:headerValue="Bearer {}" o:resourceId="https://service.[Flow.BaseHost]/" o:authorityUrl="https://login.windows.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="PAPIProfileServiceGetProfile" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/profile/v1.0/me/profile</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="IAPIImageServiceGetPhoto" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/profile/v1.0/me/image/resize%28width%3D384%2Cheight%3D384%2CallowResizeUp%3Dfalse%29</o:url>
   <o:ticket o:policy="MBI_SSL" o:idprovider="1" o:target="[Substrate.AuthHost]" o:headerValue="Passport1.4 from-PP='{}&amp;p='" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://[Substrate.AuthHost]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="PinRedemptionAADEndpoint">
   <o:url>https://login.microsoftonline.com/msretailfederation.onmicrosoft.com/oauth2/token</o:url>
  </o:service>
  <o:service o:name="PinRedemptionAADResource">
   <o:url>https://redemptionservices.microsoft.com/</o:url>
  </o:service>
  <o:service o:name="OfficeRecommendationsServiceBaseURL" o:authentication="1">
   <o:url>https://ofcrecsvcapi-int.azurewebsites.net/</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="ssl.[Live.WebHost]" o:headerValue="{}" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="AADTokenRedemption">
   <o:url>https://login.microsoftonline.com/msretailfederation.onmicrosoft.com/oauth2/token</o:url>
  </o:service>
  <o:service o:name="MroDeviceMgrC2R">
   <o:url>https://mrodevicemgr.[OSI.BaseHost]/mrodevicemgrsvc/api</o:url>
  </o:service>
  <o:service o:name="MroDeviceMgrSdx">
   <o:url>https://mrodevicemgr.[OSI.BaseHost]/mrodevicemgrsvc/api</o:url>
  </o:service>
  <o:service o:name="ODCLogging">
   <o:url>https://[OMEX.BaseTemplateHost]/client/log</o:url>
  </o:service>
  <o:service o:name="AppAcquisitionLogging">
   <o:url>https://[OMEX.BaseHost]/app/acquisitionlogging</o:url>
  </o:service>
  <o:service o:name="AddInEmailTemplate">
   <o:url>https://[OMEX.BaseHost]/api/addins/emailtemplate</o:url>
  </o:service>
  <o:service o:name="AddInsPowerBIInClientStore">
   <o:url>https://[OMEX.BaseHost]/gyro/powerbiclient</o:url>
  </o:service>
  <o:service o:name="AddInsWXPInClientStore">
   <o:url>https://[OMEX.BaseHost]/gyro/clientstore</o:url>
  </o:service>
  <o:service o:name="AddInsInClientStore" o:authentication="1">
   <o:url>https://[OMEX.BaseHost]/gyro/client</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" o:parameterName="t" />
  </o:service>
  <o:service o:name="AwsCgQuery">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/gallery</o:url>
  </o:service>
  <o:service o:name="AppDownload">
   <o:url>https://[OMEX.BaseHost]/app/download</o:url>
  </o:service>
  <o:service o:name="AppInfoQuery15">
   <o:url>https://[OMEX.BaseHost]/appinfo/query</o:url>
  </o:service>
  <o:service o:name="AppInstallInfoQuery15">
   <o:url>https://[OMEX.BaseHost]/appinstall/unauthenticated</o:url>
  </o:service>
  <o:service o:name="AppLandingPage" o:authentication="1">
   <o:url>https://[OMEX.BaseHost]/webapplandingpage.aspx</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" o:parameterName="t" />
  </o:service>
  <o:service o:name="AppQuery15">
   <o:url>https://[OMEX.BaseHost]/app/query</o:url>
  </o:service>
  <o:service o:name="AppStateQuery15">
   <o:url>https://[OMEX.BaseHost]/appstate/query</o:url>
  </o:service>
  <o:service o:name="ClientAppInstallInfoQuery15" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/appinstall/authenticated</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="ClientAppInstallInfoQueryOrgId" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/orgid/appinstall/authenticated</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="c606301c-f764-4e6b-aa45-7caaaea93c9a" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="CommerceQuery15">
   <o:url>https://[OMEX.BaseHost]/commerce/query</o:url>
  </o:service>
  <o:service o:name="DeepLinkingService">
   <o:url>https://store.office.com/addinstemplate</o:url>
  </o:service>
  <o:service o:name="DeepLinkingServiceBlackForest">
   <o:url>https://store.office.de/addinstemplate</o:url>
  </o:service>
  <o:service o:name="DeepLinkingServiceChina">
   <o:url>https://store.office.cn/addinstemplate</o:url>
  </o:service>
  <o:service o:name="DeepLinkingServiceDogfood">
   <o:url>https://store.officeppe.com/addinstemplate</o:url>
  </o:service>
  <o:service o:name="DeepLinkingTemplates">
   <o:url>https://omextemplates.content.office.net/support/templates</o:url>
  </o:service>
  <o:service o:name="EntitlementQuery15" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/entitlement/query</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="EntitlementQueryOrgId" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/orgid/entitlement/query</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="LastStoreCatalogUpdate" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/catalog/laststoreupdate</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="MyApps" o:authentication="1">
   <o:url>https://[OMEX.BaseHost]/myapps.aspx</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" o:parameterName="t" />
  </o:service>
  <o:service o:name="OutlookWebStore" o:authentication="1">
   <o:url>https://store.office.com/?productgroup=Outlook</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" o:parameterName="t" />
  </o:service>
  <o:service o:name="RemoveApps" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/apps/remove</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
  </o:service>
  <o:service o:name="RemoveAppsOrgId" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/orgid/apps/remove</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ReviewQuery15">
   <o:url>https://[OMEX.BaseHost]/review/query</o:url>
  </o:service>
  <o:service o:name="TemplateStart">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/start</o:url>
  </o:service>
  <o:service o:name="TemplateStartMac">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/mac15/start</o:url>
  </o:service>
  <o:service o:name="TemplateSearchMac">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/mac15/search</o:url>
  </o:service>
  <o:service o:name="TemplateStartModern">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/modern/start</o:url>
  </o:service>
  <o:service o:name="TemplateSearch">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/search</o:url>
  </o:service>
  <o:service o:name="TemplateSearchModern">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/modern/search</o:url>
  </o:service>
  <o:service o:name="TemplateDetails">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/details</o:url>
  </o:service>
  <o:service o:name="TemplateDetailsModern">
   <o:url>https://[OMEX.BaseTemplateHost]/client/templates/modern/details</o:url>
  </o:service>
  <o:service o:name="AirTrafficControlRules" o:authentication="1">
   <o:url>https://[OMEX.BaseMessagingHost]/airtrafficcontrol/governancerules</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://messaging.office.com/*" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="GetCustomMessage" o:authentication="2">
   <o:url>https://[OMEX.BaseMessagingHost]/lifecycle/getcustommessage16</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://messaging.office.com/*" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="LegacyGetCustomMessage">
   <o:url>https://[OMEX.BaseMessagingHost]/lifecycle/legacygetcustommessage16</o:url>
  </o:service>
  <o:service o:name="StoreUserStatus" o:authentication="2">
   <o:url>https://[OMEX.BaseHost]/storeuserstatus/api/storeuserstatus</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="LegacySetUserAction">
   <o:url>https://[OMEX.BaseMessagingHost]/lifecycle/legacysetuseraction16</o:url>
  </o:service>
  <o:service o:name="SendAutoRenewAction">
   <o:url>https://[OMEX.BaseMessagingHost]/lifecycle/SendAutoRenewAction</o:url>
  </o:service>
  <o:service o:name="SetUserAction" o:authentication="2">
   <o:url>https://[OMEX.BaseMessagingHost]/lifecycle/setuseraction16</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OMEX.AuthHost]" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="https://messaging.office.com/*" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="FirstPartyAppCDN">
   <o:url>https://contentstorage.[OSI.ONETBaseHost]/%s</o:url>
  </o:service>
  <o:service o:name="FirstPartyAppQuery">
   <o:url>https://[OMEX.BaseHost]/firstpartyapp/query</o:url>
  </o:service>
  <o:service o:name="TemplateStartOrgId" o:authentication="2">
   <o:url>https://[OMEX.BaseTemplateHost]/orgid/client/templates/start</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="ReadyServiceUrl">
   <o:url>https://ready.[OSI.ONETBaseHost]/orfo</o:url>
  </o:service>
  <o:service o:name="OneNoteApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/api</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteAugmentationEntitySearchApi">
   <o:url>https://[OneNote.BaseHost]/onaugmentation/entitysearch</o:url>
  </o:service>
  <o:service o:name="OneNoteCloudFilesApi">
   <o:url>https://cloudfiles.onenote.com/upload.aspx</o:url>
  </o:service>
  <o:service o:name="OneNoteCloudFilesConsumerEmbed">
   <o:url>https://onedrive.live.com/embed?</o:url>
  </o:service>
  <o:service o:name="OneNoteHierarchySyncWebSocket" o:authentication="2">
   <o:url>wss://hierarchyapi.[OneNote.BaseHostRootDomain]/hierarchy/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteHierarchySyncWebSocketWUS2" o:authentication="2">
   <o:url>wss://wus2-000.hierarchyapi.[OneNote.BaseHostRootDomain]/hierarchy/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteNotebooksApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/api/v1.0/me/notes/notebooks</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteNotebookShareApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/notebooks/share</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteOEmbedApi">
   <o:url>https://[OneNote.BaseHost]/externalcontent/embed</o:url>
  </o:service>
  <o:service o:name="OneNoteOEmbedProvidersApi">
   <o:url>https://[OneNote.BaseHost]/externalcontent/providers</o:url>
  </o:service>
  <o:service o:name="OneNoteRealtimeSyncHub" o:authentication="2">
   <o:url>https://devnull.onenote.com</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteRealtimeSyncHubPPE" o:authentication="2">
   <o:url>https://devnull.onenote.com</o:url>
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="https://login.windows-ppe.net/common/oauth2/authorize" />
  </o:service>
  <o:service o:name="OneNoteSharedPagesSyncApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/pages/shared/</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
  </o:service>
  <o:service o:name="OneNoteSyncApi" o:authentication="2">
   <o:url>https://contentsync.[OneNote.BaseHostRootDomain]/contentsync/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncApiDogfood" o:authentication="2">
   <o:url>https://wus2-000.contentsync.[OneNote.BaseHostRootDomain]/contentsync/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncAttachment" o:authentication="2">
   <o:url>https://pagecontentsync.[OneNote.BaseHostRootDomain]/pagecontentsync/attachment/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncAttachmentDogfood" o:authentication="2">
   <o:url>https://wus2-000.pagecontentsync.[OneNote.BaseHostRootDomain]/pagecontentsync/attachment/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncGetMissingAttachmentIds" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/v1/attachment/GetMissingAttachmentIds</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncInvitationsApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/v1/invitations</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
  </o:service>
  <o:service o:name="OneNoteSyncMembershipsApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/v1/memberships</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
  </o:service>
  <o:service o:name="OneNoteSyncPagesApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/v1/pages</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
  </o:service>
  <o:service o:name="OneNoteSyncUsersApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/sync/v1/users</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
  </o:service>
  <o:service o:name="OneNoteSyncWebSocket" o:authentication="2">
   <o:url>wss://contentsync.[OneNote.BaseHostRootDomain]/contentsync/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncWebSocketDogfood" o:authentication="2">
   <o:url>wss://wus2-000.contentsync.[OneNote.BaseHostRootDomain]/contentsync/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteSyncWebSocketTeamDogfood" o:authentication="2">
   <o:url>wss://ncus-000.contentsync.[OneNote.BaseHostRootDomain]/contentsync/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteWhiteboardApi" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/wb/api</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteRealtimeChannel" o:authentication="2">
   <o:url>https://realtimesync.[OneNote.BaseHostRootDomain]/realtimechannel/v1.0/signalr/hub</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteEducationUser" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/userinfo/v1/IsEducationUser</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteUserInfoSetting" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/userinfo/v1/setting</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteUserInfoWhoIs" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/userinfo/v1/whois</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteTextYourself">
   <o:url>https://asgsmsproxyapi.azurewebsites.net/</o:url>
  </o:service>
  <o:service o:name="OneNoteBulletins">
   <o:url>https://[OneNote.BaseHost]/bulletins</o:url>
  </o:service>
  <o:service o:name="OneNoteGeoLocation" o:authentication="2">
   <o:url>https://lookup.onenote.com/lookup/geolocation/v1</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteMyPagesAPI" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/api/v1.0/me/notes/pages</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteNotificationRegister" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/NotificationsAPI/Register</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNotePremiumFeatures" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/userinfo/v1/settings/IsFeatureEnabled/PremiumFeatures</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="OneNoteUserTypes" o:authentication="2">
   <o:url>https://[OneNote.BaseHost]/userinfo/v1/UserTypes</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="StickyNotesClient" o:authentication="2">
   <o:url>https://[Substrate.BaseHost]/NotesClient</o:url>
   <o:ticket o:policy="MBI_SSL_SHORT" o:idprovider="1" o:target="[OneNote.AuthHost]" o:headerValue="WLID1.0 {}&amp;p=" />
   <o:ticket o:idprovider="3" o:headerValue="Bearer {}" o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:service>
  <o:service o:name="LiveOAuthLoginStart">
   <o:url>https://login.[Live.WebHost]/oauth20_authorize.srf?client_id=[LiveOAuthAppID]&amp;response_type=token&amp;redirect_uri=https://login.[Live.WebHost]/oauth20_desktop.srf&amp;noauthcancel=1</o:url>
  </o:service>
  <o:service o:name="LiveOAuthSignUp">
   <o:url>https://signup.[Live.WebHost]/signup?ru=https://login.[Live.WebHost]/oauth20_authorize.srf%3fclient_id%3d[LiveOAuthAppID]%26scope%3dservice::ssl.[Live.WebHost]::MBI_SSL_SHORT%26response_type%3dtoken%26redirect_uri%3dhttps://login.[Live.WebHost]/oauth20_desktop.srf%26display%3dtouch&amp;uiflavor=host&amp;lic=1&amp;wsucxt=2</o:url>
  </o:service>
 </o:services>
 <o:tokens>
  <o:token o:name="EXO.BaseHost">outlook.office365.com</o:token>
  <o:token o:name="Live.DocumentAPIHost">docs.live.net</o:token>
  <o:token o:name="Live.FederatedAuthHost">urn:federation:MicrosoftOnline</o:token>
  <o:token o:name="Live.WebHost">live.com</o:token>
  <o:token o:name="TestInProd.WebHost">officeappsint.com</o:token>
  <o:token o:name="TestInProdRoaming.WebHost">officeapps.live.com</o:token>
  <o:token o:name="MAX.AuthHost">office.com</o:token>
  <o:token o:name="MAX.BaseHost">support.office.com</o:token>
  <o:token o:name="MAX.ResourceId">4b233688-031c-404b-9a80-a4f3f2351f90</o:token>
  <o:token o:name="MSN.WebHost">msn.com</o:token>
  <o:token o:name="ODC.AuthHost">[ODC.BaseHost]</o:token>
  <o:token o:name="ODC.BaseHost">office.microsoft.com</o:token>
  <o:token o:name="ODC.CDNBaseHost">officeimg.vo.msecnd.net</o:token>
  <o:token o:name="OMEX.AuthHost">office.com</o:token>
  <o:token o:name="OMEX.BaseCoSubHost">stores.office.com</o:token>
  <o:token o:name="OMEX.BaseHost">store.office.com</o:token>
  <o:token o:name="OMEX.BaseMessagingHost">messaging.office.com</o:token>
  <o:token o:name="OMEX.BaseTemplateHost">templateservice.office.com</o:token>
  <o:token o:name="OneDriveApi.BaseHost">api.onedrive.com</o:token>
  <o:token o:name="OSI.BaseHost">officeapps.live.com</o:token>
  <o:token o:name="OSI.BaseHost.Global">officeapps.live.com</o:token>
  <o:token o:name="OSI.ONETBaseHost">osi.office.net</o:token>
  <o:token o:name="OSI.RootHost">officeapps.live.com</o:token>
  <o:token o:name="RMSOnline.BaseHost">discover.aadrm.com</o:token>
  <o:token o:name="SPO.BaseHost">sharepoint.com</o:token>
  <o:token o:name="Forms.BaseHost">forms.office.com</o:token>
  <o:token o:name="Substrate.AuthHost">substrate.office.com</o:token>
  <o:token o:name="Substrate.BaseHost">substrate.office.com</o:token>
  <o:token o:name="Outlook.AuthHost">outlook.office.com</o:token>
  <o:token o:name="Flow.AuthHost">https://service.[Flow.BaseHost]/</o:token>
  <o:token o:name="Flow.BaseHost">flow.microsoft.com</o:token>
  <o:token o:name="Teams.BaseHost">teams.microsoft.com</o:token>
  <o:token o:name="AADRedemptionServiceResourceId">https://redemptionservices.microsoft.com/</o:token>
  <o:token o:name="ACSClientSecret">gI0PLmGIUVrSRNOZXf1EeQKzZ0JDTWDUbBV%2bL15%2bjzU%3d</o:token>
  <o:token o:name="ACSHost">posarprodcssservice.accesscontrol.windows.net</o:token>
  <o:token o:name="ACSScope">http%3a%2f%2fredemptionservices.microsoft.com%2f</o:token>
  <o:token o:name="ADALAuthorityUrl">https://login.windows.net/common/oauth2/authorize</o:token>
  <o:token o:name="ADALClientIdExcel">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdLync">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdOffice">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdOneNote">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdOMEX">c606301c-f764-4e6b-aa45-7caaaea93c9a</o:token>
  <o:token o:name="ADALClientIdOutlook">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdPowerPoint">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdProject">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdRMS">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdSkyDrivePro">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdVisio">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALClientIdWord">d3590ed6-52b3-4102-aeff-aad2292ab01c</o:token>
  <o:token o:name="ADALPPEAuthorityUrl">https://login.windows-ppe.net/common/oauth2/authorize</o:token>
  <o:token o:name="ADALRedirectUrlExcel">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlLync">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlOneNote">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlOutlook">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlPowerPoint">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlProject">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlRMS">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlSkyDrivePro">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlVisio">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALRedirectUrlWord">urn:ietf:wg:oauth:2.0:oob</o:token>
  <o:token o:name="ADALResourceId">https://officeapps.live.com</o:token>
  <o:token o:name="BingGeospatialServiceKey">AiUYDtsomloBulT2jUfFPb_J0tIqjpPTMfuXbgvzYVdcoGZBprH9G3yrfVMYEbkN</o:token>
  <o:token o:name="BingGeospatialServiceKey3DMaps">AutmxuJvVVVQyluwfF-Le9A6WQ_ypucXcJbzx5Rwf5u8on47kJRDu19BzV4kZlq9</o:token>
  <o:token o:name="BingSpellerServiceAuthenticationId">FDD7CBDCD2C7BC531CBFCDB30B10E75034BDEC74</o:token>
  <o:token o:name="BingSpellerServiceSupportedLanguages">en-us</o:token>
  <o:token o:name="BlackforestFederationDomain">microsoftonline.de</o:token>
  <o:token o:name="DSCRedemptionHost">res.getmicrosoftkey.com</o:token>
  <o:token o:name="EnableOutlookInClientStore">true</o:token>
  <o:token o:name="EnablePhotoFromMsGraph">False</o:token>
  <o:token o:name="EnablePPTBinaryDecryption">true</o:token>
  <o:token o:name="EnableOutlookInClientStoreOnPrem">false</o:token>
  <o:token o:name="EnablePPTImmersivePrint">true</o:token>
  <o:token o:name="EnableSuggestions">false</o:token>
  <o:token o:name="GraphImportFlights">Text=None;;Pdf=None;;LocalTap=None;;ReuseScoreModel=None;;ZeroTermThroughThreeS=None;;ReuseScoreWithPasteInfo=None;;</o:token>
  <o:token o:name="GraphImportGalleryTableHeight">140</o:token>
  <o:token o:name="GraphImportGalleryTableCount">1</o:token>
  <o:token o:name="GraphImportGoLocalExpiredHours">720</o:token>
  <o:token o:name="GraphImportHtmlExpiredHours">24</o:token>
  <o:token o:name="GraphImportPackageExpiredHours">24</o:token>
  <o:token o:name="GraphImportPackageUrlFormat">api/package?tenant={0}&amp;lcid={1}&amp;targetType={2}&amp;zeroTermPackage=false&amp;objectFilterPackage=true&amp;objectStream=true&amp;objectFilter=Table&amp;objectCountForFilter=1</o:token>
  <o:token o:name="GraphImportQFWarmUpUrl">https://outlook.office.com/autosuggest/api/v1/init?cvid={0}&amp;scenario=saturnsearch</o:token>
  <o:token o:name="IsDelveEnabled_UrlFormat">https://shredder.[OSI.ONETBaseHost]/ShredderService/web/static-resources/IsDelveEnabled.txt</o:token>
  <o:token o:name="IsDelveEnabled_RegexToParseJson">a</o:token>
  <o:token o:name="IsDelveEnabled_RegexToParseUserOptOut">\"Status\":[4],</o:token>
  <o:token o:name="TapSearchUrlFormat">api/search/search?tenant={0}&amp;lcid={1}&amp;querytext={2}&amp;rowLimit=30&amp;objectTypeFilter={3}&amp;isEchoScenario=false</o:token>
  <o:token o:name="OutlookInClientPrivateAppsMinExchangeVersion">15.1.517.0</o:token>
  <o:token o:name="OutlookInClientStoreMinExchangeVersion">15.1.354.1</o:token>
  <o:token o:name="OutlookConnectorsSenderWhitelist">outlook@email2.office.com,reply@email2.office.com</o:token>
  <o:token o:name="OutSpaceFileInfoEnableDynamicCanvas">True</o:token>
  <o:token o:name="OutSpaceProfileEnableDynamicCanvas">True</o:token>
  <o:token o:name="PersonalizationEnableInsights">True</o:token>
  <o:token o:name="PersonalizationEnableSignals">True</o:token>
  <o:token o:name="PersonalizationEnableUserActions">True</o:token>
  <o:token o:name="PinRedemptionAADClientId">06c6433f-4fb8-4670-b2cd-408938296b8e</o:token>
  <o:token o:name="PinRedemptionAADClientSecret">ia6LkjM5TSWFnL8isioR4iw0ld9h2jP6eNOfM8W1ruI=</o:token>
  <o:token o:name="PinRedemptionAADHost">login.microsoftonline.com</o:token>
  <o:token o:name="SharepointFilesHostFormat">{tenant}-files{host}</o:token>
  <o:token o:name="SharepointMyFilesHostFormat">{tenant}files{host}</o:token>
  <o:token o:name="SuggestionAbortRequestTimeMS">1000</o:token>
  <o:token o:name="SuggestionAuthFlags">0</o:token>
  <o:token o:name="SuggestionMaxRequestSize">2097152</o:token>
  <o:token o:name="SuggestionDisplayPaneTimeOutMS">5000</o:token>
  <o:token o:name="Suggestion4x3Enabled">true</o:token>
  <o:token o:name="SuggestionVectorImagesEnabled">true</o:token>
  <o:token o:name="SuggestionMultipleObjectsPerSlideEnabled">true</o:token>
  <o:token o:name="SuggestionThemesEnabled">7ACABC62-BF99-48CF-A9DC-4DB89C7B13DC;71A07785-5930-41D4-9A83-E23602B48E98;98DFF888-2449-4D28-977C-6306C017633E;5665723A-49BA-4B57-8411-A56F8F207965;7B5DBA9E-B069-418E-9360-A61BDD0615A4;C4BB2A3D-0E93-4C5F-B0D2-9D3FCE089CC5;EC9488ED-E761-4D60-9AC4-764D1FE2C171;7BEAFC2A-325C-49C4-AC08-2B765DA903F9;8984A317-299A-4E50-B45D-BFC9EDE2337A;C0C680CD-088A-49FC-A102-D699147F32B2;EEC9B30E-2747-4D42-BCBE-A02BDEEEA114;3841520A-25F2-4EB8-BE4C-611DB5ABEED9;3577F8C9-A904-41D8-97D2-FD898F53F20E;FC33163D-4339-46B1-8EED-24C834239D99;B8441ADB-2E43-4AF7-B97A-BD870242C6A8;AC372BB4-D83D-411E-B849-B641926BA760;789EC3FE-34FD-429C-9918-760025E6C145;4C5440D6-04D2-4954-96CF-F251137069B2;28CDC826-8792-45C0-861B-85EB3ADEDA33;3388167B-A2EB-4685-9635-1831D9AEF8C4;39EC5628-30ED-4578-ACD8-9820EDB8E15A;5F128B03-DCCA-4EEB-AB3B-CF2899314A46;1306E473-ED32-493B-A2D0-240A757EDD34;C3F70B94-7CE9-428E-ADC1-3269CC2C3385;0507925B-6AC9-4358-8E18-C330545D08F8;4FDF2955-7D9C-493C-B9F9-C205151B46CD;BA0EB5A6-F2D4-4F82-977B-64ADEE4A2A69;7CB32D59-10C0-40DD-B7BD-2E94284A981C;62F939B6-93AF-4DB8-9C6B-D6C7DFDC589F;0AC2F7E7-15F5-431C-B2A2-456FE929F56C;F9A299A0-33D0-4E0F-9F3F-7163E3744208;9697A71B-4AB7-4A1A-BD5B-BB2D22835B57;F226E7A2-7162-461C-9490-D27D9DC04E43;BBFCD31E-59A1-489D-B089-A3EAD7CAE12E;8BEC4385-4EB9-4D53-BFB5-0EA123736B6D;972BB86E-8AFD-4DE8-800F-A08F6EC4E608;B844A9D6-DE23-412A-B497-F6E208B22C16;309C13C0-3BE0-4E8F-8916-1D5516B3B5DD;18E1BE87-7240-45DF-8788-3CAEB7F17AB1;F9915BBD-9749-466F-995C-8C8D6A938EC0;CF1D1A65-FC75-42D2-B7EF-D2991382DC6F;A55DF1DA-22EC-4DA4-B170-D3F0FF81047C;3BFA2149-51D1-489C-9B65-4F9563B089DE;9B55E993-63C4-4E9B-9466-30BCDDC6903B;C2EC3228-ECB7-4E58-8F51-112F019FC712;74669713-C080-4C4B-A409-6275C5640D79;1684E3E2-D34F-4E98-BFE5-F03BFC5861D2;ED3996BA-162B-43C7-B0E2-A5CA4E649741;187088E4-27D7-4455-856F-4A44258D82E2</o:token>
  <o:token o:name="SuggestionJPEGConversionEnabled">false</o:token>
  <o:token o:name="SuggestionKnownPartsExpirationInMinutes">28</o:token>
  <o:token o:name="SuggestionMinTypingIdleTimeMS">0</o:token>
  <o:token o:name="GallatinFederationDomain">partner.microsoftonline.cn</o:token>
  <o:token o:name="GlobalFederationProvider">microsoftonline.com</o:token>
  <o:token o:name="LiveIdTrustedRootDomains">live.net|live.com|live.org|onedrive.com|office.com|office365.com|outlook.com|outlook-sdf.com|hotmail.com</o:token>
  <o:token o:name="LiveOAuthAppID">00000000480728C5</o:token>
  <o:token o:name="LiveProfileViewName">Office15.Profile</o:token>
  <o:token o:name="NLProofingServiceTimeout">670</o:token>
  <o:token o:name="OfficeIdentityVersion">0</o:token>
  <o:token o:name="SSExcelCSMaxConvertibleSize">41943040</o:token>
  <o:token o:name="SSExcelPSMaxConvertibleSize">41943040</o:token>
  <o:token o:name="SSPPTCSMaxConvertibleSize">41943040</o:token>
  <o:token o:name="SSPPTPSMaxConvertibleSize">41943040</o:token>
  <o:token o:name="SSWordCSMaxConvertibleSize">41943040</o:token>
  <o:token o:name="SSWordPSMaxConvertibleSize">41943040</o:token>
  <o:token o:name="SSMaxTime">300000</o:token>
  <o:token o:name="SSMaxRetries">10</o:token>
  <o:token o:name="SSDefaultProxyTimeout">900000</o:token>
  <o:token o:name="SSEnableAuth">true</o:token>
  <o:token o:name="SSEnableE1OTransferToService">false</o:token>
  <o:token o:name="SSEnableCache">true</o:token>
  <o:token o:name="SSEnableCacheXL">false</o:token>
  <o:token o:name="SSEnableCacheXLNew">true</o:token>
  <o:token o:name="SSCacheMaxSizeInBytes">78643200</o:token>
  <o:token o:name="SSCacheMaxSizeLowDiskDeviceInBytes">36700160</o:token>
  <o:token o:name="SSCacheMaxSizeXLInBytes">26214400</o:token>
  <o:token o:name="SSCacheMaxSizeLowDiskDeviceXLInBytes">15728640</o:token>
  <o:token o:name="SSCacheMaxDaysOld">14</o:token>
  <o:token o:name="SSCacheMaxFileCount">50</o:token>
  <o:token o:name="SSCacheMaxFileSizeToHashInBytes">78643200</o:token>
  <o:token o:name="SSEnableCacheWord">true</o:token>
  <o:token o:name="SSCacheMaxSizeWordInBytes">26214400</o:token>
  <o:token o:name="SSCacheMaxSizeLowDiskDeviceWordInBytes">15728640</o:token>
  <o:token o:name="ADALPBIResourceId">https://analysis.windows.net/powerbi/api</o:token>
  <o:token o:name="MruMaxLocalItemCount">100</o:token>
  <o:token o:name="EnableAutoRenewOutSpace">true</o:token>
  <o:token o:name="EnableXL2PBIFullFidelity">true</o:token>
  <o:token o:name="EnableXL2PBILocalFiles">true</o:token>
  <o:token o:name="XL2PBIMaxFileSize">250</o:token>
  <o:token o:name="XL2PBIMaxNonModelPartSize">10</o:token>
  <o:token o:name="EnableTargetedMessagingBusBar">true</o:token>
  <o:token o:name="EnableTargetedMessagingDisplayOnce">true</o:token>
  <o:token o:name="EnableTargetedMessagingOutSpace">true</o:token>
  <o:token o:name="GPWord">AlphaArm:2001375553,AlphaX86:2001375557,BetaArm:2001364277,BetaX86:2001364281,GAArm:2001364271,GAX86:2001364275</o:token>
  <o:token o:name="GPExcel">AlphaArm:2001375553,AlphaX86:2001375557,BetaArm:2001364277,BetaX86:2001364281,GAArm:2001364271,GAX86:2001364275</o:token>
  <o:token o:name="GPPPT">AlphaArm:2001375553,AlphaX86:2001375557,BetaArm:2001364277,BetaX86:2001364281,GAArm:2001364271,GAX86:2001364275</o:token>
  <o:token o:name="GPOneNote">AlphaArm:0,AlphaX86:0,BetaArm:0,BetaX86:0,GAArm:1568214825,GAX86:1801344673</o:token>
  <o:token o:name="OfficeAppsEnableLog">Channel:beta,AllUsers:true,App:all,LiveIDs:,GoogleIDs:,DeviceIDs:,LogLevel:info</o:token>
  <o:token o:name="EnableTargetedMessagingBusBarCanvas">true</o:token>
  <o:token o:name="EnableTargetedMessagingCanvasBoot">true</o:token>
  <o:token o:name="EnableTargetedMessagingCanvasSave">true</o:token>
  <o:token o:name="DynamicCanvasContentUrl">https://[OMEX.BaseCoSubHost]/canvas/markup</o:token>
  <o:token o:name="MsGraphBaseURL">graph.microsoft.com</o:token>
  <o:token o:name="MsGraphVersion">v1.0</o:token>
  <o:token o:name="MsGraphRegionCheck">true</o:token>
  <o:token o:name="UpdateNotifierWord">VersionARM:2001406271,DateARM:2017-06-06,VersionX86:2001406275,DateX86:2017-06-06</o:token>
  <o:token o:name="UpdateNotifierExcel">VersionARM:2001406271,DateARM:2017-06-06,VersionX86:2001406275,DateX86:2017-06-06</o:token>
  <o:token o:name="UpdateNotifierPowerPoint">VersionARM:2001406271,DateARM:2017-06-06,VersionX86:2001406275,DateX86:2017-06-06</o:token>
  <o:token o:name="AADGraphBaseURL">graph.windows.net</o:token>
  <o:token o:name="CentennialPkgFamilyNameBase">ProjectCentennialDogfood.29088C9C30B23_mbb0nbh50f13t</o:token>
  <o:token o:name="CentennialPDPBase">ms-windows-store://pdp/?ProductId=CFQ7TTC0K5BF&amp;referrer=Office.GetOfficeApp</o:token>
  <o:token o:name="CentennialRateAndReview">ms-windows-store://review/?ProductId=</o:token>
  <o:token o:name="WAMProviderDefaultUrl">https://login.windows.local</o:token>
  <o:token o:name="WAMProviderAADUrl">https://login.microsoft.com</o:token>
  <o:token o:name="WAMProviderMSAUrl">https://login.microsoft.com</o:token>
  <o:token o:name="EnableTargetedMessagingWin32GetOfficeCarousel">true</o:token>
  <o:token o:name="OfficeEntityVersioniOS">2.3</o:token>
  <o:token o:name="ASGProxySubscriptionKey">0f6d01429f3e4224825d317560c7b28b</o:token>
  <o:token o:name="EquivalentAuthorityHosts">login.windows.net|login.microsoftonline.com</o:token>
  <o:token o:name="PlannerMobileMinVersion">v1</o:token>
  <o:token o:name="ResearcherAutoLaunchWikiPasteLangs">^(en|fr|it|de|es|ja)\.wikipedia\.org$</o:token>
  <o:token o:name="BingSpeechApiSubscriptionKey">8a7ae2f43fc845749b577bf4b01a4903</o:token>
  <o:token o:name="EditorServiceTimeout">670</o:token>
  <o:token o:name="BingSpeechSupportedLanguages">en-US;es-ES;zh-CN;en-CA;es-MX</o:token>
  <o:token o:name="BingSpeechSupportedLanguagesPreview">de-DE;fr-FR;it-IT;en-AU;en-IN;en-GB;fr-CA;pt-BR</o:token>
  <o:token o:name="SharePointGraphAPIUrlSuffix">/_api/v2.0</o:token>
  <o:token o:name="PowerQueryDataCatalogSearchEnabled">true</o:token>
  <o:token o:name="PowerQueryMicrosoftExchangeConnectorEnabled">true</o:token>
  <o:token o:name="TokenAriaUploadOnly">False</o:token>
  <o:token o:name="TokenDisableAllTelemetry">False</o:token>
  <o:token o:name="TokenTelemetryRestrictedSovereigns">microsoftonline.us;microsoftonline.mil</o:token>
  <o:token o:name="IsITARToken">False</o:token>
  <o:token o:name="TokenIdeasEnabled">true</o:token>
  <o:token o:name="FederationProviderToken">1</o:token>
  <o:token o:name="CalendarsJapaneseEras">1868_01_01_ÊòéÊ≤ª_Êòé_Meiji_M|1912_07_30_Â§ßÊ≠£_Â§ß_Taisho_T|1926_12_25_Êò≠Âíå_Êò≠_Showa_S|1989_01_08_Âπ≥Êàê_Âπ≥_Heisei_H</o:token>
  <o:token o:name="CalendarsJapaneseErasTest1">1868_01_01_ÊòéÊ≤ª_Êòé_Meiji_M|1912_07_30_Â§ßÊ≠£_Â§ß_Taisho_T|1926_12_25_Êò≠Âíå_Êò≠_Showa_S|1989_01_08_Âπ≥Êàê_Âπ≥_Heisei_H|2011_02_02_Aa_A_Aaaaaa_A|2019_05_01_Bb_B_Bbbbbb_B|2019_11_11_Cc_C_Cccccc_C</o:token>
  <o:token o:name="CalendarsJapaneseErasTest2">1868_01_01_ÊòéÊ≤ª_Êòé_Meiji_M|1912_07_30_Â§ßÊ≠£_Â§ß_Taisho_T|1926_12_25_Êò≠Âíå_Êò≠_Showa_S|1989_01_08_Âπ≥Êàê_Âπ≥_Heisei_H|2019_05_01_Zz_Z_Zzzzzz_Z</o:token>
  <o:token o:name="OMEXReadyToolkitScanning">1.0.0.0:1</o:token>
  <o:token o:name="OMEXReadyUsageScanning">1.0.0.0:1</o:token>
  <o:token o:name="BlockedFileExtensionsList">ade;adp;app;appcontent-ms;application;asp;bas;bat;bgi;cer;chm;cmd;cnt;com;cpl;crt;csh;der;diagcab;exe;fxp;gadget;grp;hlp;hpj;hta;inf;ins;iso;isp;its;jar;jnlp;js;jse;ksh;lnk;mad;maf;mag;mam;maq;mar;mas;mat;mau;mav;maw;mcf;mda;mdb;mde;mdt;mdw;mdz;msc;msh;msh1;msh2;msh1xml;msh2xml;mshxml;msi;msp;mst;msu;ops;osd;pcd;pif;pl;plg;prf;prg;printerexport;ps1;ps2;ps1xml;ps2xml;psc1;psc2;psd1;psdm1;pst;reg;scf;scr;sct;settingcontent-ms;shb;shs;theme;tmp;url;vb;vbe;vbp;vbs;vsmacros;vsw;webpnp;website;ws;wsc;wsf;wsh;xbap;xll;xnk</o:token>
  <o:token o:name="OneNote.AuthHost">onenote.com</o:token>
  <o:token o:name="OneNote.BaseHost">www.onenote.com</o:token>
  <o:token o:name="OneNote.BaseHostRootDomain">onenote.com</o:token>
 </o:tokens>
 <o:signin>
  <o:tickets o:idprovider="1">
   <o:ticket o:policy="MBI_SSL_SHORT" o:target="ssl.[Live.WebHost]" />
   <o:ticket o:policy="MBI_SSL_SHORT" o:target="officeapps.[Live.WebHost]" />
  </o:tickets>
  <o:tickets o:idprovider="2">
   <o:ticket o:policy="MBI" o:target="[SPO.BaseHost]" />
   <o:ticket o:policy="MBI" o:target="[OSI.RootHost]" />
  </o:tickets>
  <o:tickets o:idprovider="3">
   <o:ticket o:resourceId="[ADALResourceId]" o:authorityUrl="[ADALAuthorityUrl]" />
  </o:tickets>
 </o:signin>
</o:OfficeConfig>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        y @%SystemRoot%\system32\drivers\fltmgr.sys,-10001∏ˇˇˇF i l e   S y s t e m   D e p e n d e n c y   M i n i f i l t e r   ˇˇˇlh ∏k [ã∏hbin –                        ÿˇˇˇvk    hµ      ofxa4weuuexn2q1 ÿˇˇˇvk 
   xµ      ofmu3kmdc4wpwu1 ÿˇˇˇvk 
   8∑      ofgkl2nam1jtnr1 ÿˇˇˇvk 
   Ë·      ofjdoapbffmecw1 ÿˇˇˇvk ®   ∞ö      ofrxyqkyuqn3f61 ÿˇˇˇvk l  Hœ      ofxedtvckcafnq1 ÿˇˇˇvk ¸   –ÿ      of3wb4yaou5uki1 ÿˇˇˇvk ^   «      ofpiac4jtno4nc1 ÿˇˇˇvk   x∑      ofzvlfleilyziq1 ÿˇˇˇvk    ˜      oftfdyfxd4mek31 ÿˇˇˇvk   8ƒ     e ofnb63sryndz5p1 ÿˇˇˇvk 0   X“     a ofu24jerotffmc1 ÿˇˇˇvk 0   Äí      1 of3yx25picflei1 ÿˇˇˇvk 0   µ     1 of5t2kw6gyjugb1 ÿˇˇˇvk ∏  h»     1 offenmlxtyynou1 ÿˇˇˇvk   @     1 ofuupcrewe52nj1 ÿˇˇˇvk 0   ê     1 ofhi6cxzncd22a1 ÿˇˇˇvk 0   »     1 ofxo1cjk2on5em1 ÿˇˇˇvk û   	     1 ofhdzqrprrw2f31 ÿˇˇˇvk p  ∞t     1 ofvh21m5kzltss1 ÿˇˇˇvk 0         8 ofakozhxizsdtg1 ÿˇˇˇvk	 6        o ConfigIdseyx4i1 –ˇˇˇvk ò  –	     E ULSCategoriesSeverities –ˇˇˇvk ò  `	     fgULSCategoriesSeverities ÿ   ∞ P¡  ‡ X‚ Ä‚ ®‚ –‚ ¯‚ H„ p„  „ ò„ ¿„ Ë„ ‰ 8‰ `‰ à‰ 8È `È ∞‰ ÿ‰ ∞Î  ± H± p± ò± ¿± Ë± ≤ 8≤ `≤ à≤ ∞≤ `≥ X!  4 –2 `M  u (w  x  z ∞| 0 Å @Å hÅ hÇ êÇ ∞É ÿÉ Äá ®á –á ‡à â  ã (ã ¿å 8ç »é hê êê ∏ê ÿí (ö ( P x 8 ¿ @	 ∞2 @4 § h4 ‡4 –a ¯a ± hc  b Hb pb òb ¿b Ëb c 8c êc ∏c ‡c Pg xg †g »g g h Hh ph òh (i Pi v e n t F l a g "   :   4 8 8 9 6   }    a (   vk ∏  àÖ      offenmlxtyynou1 (   vk   Ù      ofuupcre(   vk 
   êˆ ∞     oflvpls3z4oe1b1m(   vk    ˜      of1lzn5ucgjl151gp   vk ò  ∏ 	       ULSCategoriesSeverities @   ategoriesSeverities (   vk   Ä       evofbprhztalviai1 –ˇˇˇD o c u m e n t   E n c r y p t i o n   n d 0   vk p  ∞t     e ofvh21m5kzltss1v   ies –ˇˇˇU s e r   D a t a   S t o r a g e      †4 –ˇˇˇN e t w o r k   S e t u p   S e r v i c e   –ˇˇˇM i c r o s o f t   M A C   B r i d g e   s –ˇˇˇvk H   Pı     t @comres.dll,-2797   :   ÿˇˇˇN a t i v e W i F i   F i l t e r   –ˇˇˇN D I S   S y s t e m   D r i v e r     {   –ˇˇˇN e t w o r k   C o n n e c t i o n s     r ÿˇˇˇP a r t i t i o n   d r i v e r   v ÿˇˇˇP h o n e   S e r v i c e     "   : –ˇˇˇI P s e c   P o l i c y   A g e n t   m S a –ˇˇˇW A N   M i n i p o r t   ( P P T P )   a v –ˇˇˇQ o S   P a c k e t   S c h e d u l e r   8 ¿ˇˇˇvk( <   êŸ      @%systemroot%\system32\mprmsg.dll,-32012¿ˇˇˇR e m o t e   A c c e s s   I P v 6   A R P   D r i v e r   ∞ˇˇˇvk7    ®Ö       @%SystemRoot%\System32\Windows.WARP.JITService.dll,-100 ∏ˇˇˇvk, F   h⁄      @%windir%\system32\inetsrv\iisres.dll,-30001¯; ∞ˇˇˇW i n d o w s   P r o c e s s   A c t i v a t i o n   S e r v i c e    `∏ ¿ˇˇˇvk( D   ¯⁄       @%systemroot%\system32\wbengine.exe,-104∏ˇˇˇB l o c k   L e v e l   B a c k u p   E n g i n e   S e r v i c e   ¿ˇˇˇvk( 4   Ä€      @%systemroot%\system32\wbiosrvc.dll,-100»ˇˇˇW i n d o w s   B i o m e t r i c   S e r v i c e   ∏ˇˇˇvk- 8    ‹      @%systemroot%\system32\drivers\wcifs.sys,-100 g ¿ˇˇˇW i n d o w s   C o n t a i n e r   I s o l a t i o n     ¿ˇˇˇvk% P   Ä‹     \L@%SystemRoot%\system32\wcncsvc.dll,-3k' ®ˇˇˇW i n d o w s   C o n n e c t   N o w   -   C o n f i g   R e g i s t r a r     a p ∏ˇˇˇvk- L    ›      @%systemroot%\system32\drivers\wcnfs.sys,-1002\d∞ˇˇˇW i n d o w s   C o n t a i n e r   N a m e   V i r t u a l i z a t i o n      G r ∏ˇˇˇvk.     ¸     e @%SystemRoot%\system32\drivers\qwavedrv.sys,-1" ∞ˇˇˇR e m o t e   A c c e s s   A u t o   C o n n e c t i o n   M a n a g e r      "   ˇˇˇM a p s     ®ˇˇˇvk: Ä   Äﬁ     R Microsoft.Office.Shared.GrammarChecking.ItalianSubjAfterSeCitatixˇˇˇM s o : : C h e c k s u m R e g i s t r y : : D a t a | u i n t 6 4 _ t | 5 7 8 3 0 3 9 6 7 2 3 0 4 0 9 8 4 8 2 ; b o o l | 1   } , †ˇˇˇvkC Ä   8Â     " Microsoft.Office.Shared.GrammarChecking.ItalianCapitalizationPlaces vk ÿˇˇˇlh H ì§∞F uıTPI ◊ßG sQ¯ˇˇˇ¿& –ˇˇˇW A N   M i n i p o r t   ( L 2 T P )      ÿˇˇˇvk v   x4       ofc1utn6pb2vcd0 ˇˇˇlh Ë· Ô.ì6hbin   0                      xˇˇˇM s o : : C h e c k s u m R e g i s t r y : : D a t a | u i n t 6 4 _ t | 8 2 7 1 3 0 5 8 3 4 8 6 0 3 1 8 6 1 7 ; b o o l | 0   m R ®ˇˇˇvk< Ç   ÿ3      Microsoft.Office.Identity.TrustedSitesHaveVersionInUserAgent t ˇˇˇlh –ì ¿ ÔˇˇˇXì  ¿” agId   521 xˇˇˇM s o : : C h e c k s u m R e g i s t r y : : D a t a | u i n t 6 4 _ t | - 4 5 6 3 3 3 3 2 8 2 7 7 2 9 3 3 3 4 ; b o o l | 1     of¿ˇˇˇlh ËM C«⁄ÄL uıTËI ’óRHN ◊ßxw ‚tÖÜ0M sQ   ive †ˇˇˇnk  çä?Mg‘   ®£         ˇˇˇˇˇˇˇˇ   p   ˇˇˇˇ        .   ò      
   ULSMonitorSystem†ˇˇˇvkA (   »÷     c.@%SystemRoot%\system32\WindowsPowerShell\v1.0\powershell.exe,-124dll,-50àˇˇˇnk  ¢ãtÊf‘   p3        p˚ ˇˇˇˇ    ˇˇˇˇ  ˇˇˇˇ                   &   {34745C63-B2F0-4784-8B67-5E12C8701A31}7 òˇˇˇnk  ˚åæ⁄e‘   ÿ†          | ˇˇˇˇ    ˇˇˇˇ  ˇˇˇˇ                      officeclicktorun.exe.bak†ˇˇˇnk  ˚åæ⁄e‘   (Û         ˇˇˇˇˇˇˇˇ   †|   ˇˇˇˇ        .   ò      
   ULSMonitor3   5 x˛ˇˇ5 8 0 4 1 2 9 , 7 2 0 2 2 6 9 , 2 2 9 2 9 4 2 9 , 1 7 1 1 0 9 9 2 , 7 1 6 8 7 0 7 , 3 4 6 2 4 2 3 , 3 7 0 2 9 2 0 , 7 1 5 3 4 8 7 , 1 7 1 1 0 9 8 8 , 1 7 9 6 2 3 9 1 , 1 7 9 6 2 3 9 2 , 3 7 0 0 7 5 4 , 3 9 6 5 0 6 2 , 4 2 9 7 0 9 4 , 7 1 5 3 4 2 1 , 1 8 7 1 6 1 9 3 , 7 1 5 3 4 3 5 , 7 2 0 2 2 6 5 , 2 0 5 0 2 1 7 4 , 6 3 0 8 1 9 1 , 2 0 4 8 9 3 5 3 , 1 8 4 0 7 6 1 7 , 1 7 1 0 2 4 1 8   ®ˇˇˇC o n n e c t e d   D e v i c e s   P l a t f o r m   U s e r   S e r v i c e   r.dl–˛ˇˇ{   " S u b N a m e s p a c e s "   :   {   " L i n k e d I n "   :   {   " S u b N a m e s p a c e s "   :   {   " S e t t i n g s "   :   {   " E v e n t s "   :   {   " L i n k e d I n C o m p l i a n c e C h e c k "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   }   }   }   }   }   2 Ë˛ˇˇ{   " S u b N a m e s p a c e s "   :   {   " D e s k t o p "   :   {   " S u b N a m e s p a c e s "   :   {   " S y n c "   :   {   " E v e n t s "   :   {   " S y n c P u r g e O f f l i n e I t e m s "   :   {   " E v e n t F l a g "   :   2   }   }   }   }   }   }   }   òˇˇˇ" p K P X 5 v 1 j n L O d z B / s r R y 0 Q + Q x 6 g F F h U o f J V K M 1 x c v p D s = "     E v x˛ˇˇ5 8 0 4 1 2 9 , 7 2 0 2 2 6 9 , 2 2 9 2 9 4 2 9 , 1 7 1 1 0 9 9 2 , 7 1 6 8 7 0 7 , 3 4 6 2 4 2 3 , 3 7 0 2 9 2 0 , 7 1 5 3 4 8 7 , 1 7 1 1 0 9 8 8 , 1 7 9 6 2 3 9 1 , 1 7 9 6 2 3 9 2 , 3 7 0 0 7 5 4 , 3 9 6 5 0 6 2 , 4 2 9 7 0 9 4 , 7 1 5 3 4 2 1 , 1 8 7 1 6 1 9 3 , 7 1 5 3 4 3 5 , 7 2 0 2 2 6 5 , 2 0 5 0 2 1 7 4 , 6 3 0 8 1 9 1 , 2 0 4 8 9 3 5 3 , 1 8 4 0 7 6 1 7 , 1 7 1 0 2 4 1 8   X  e n t s "   :   {   " M o d e l 3 D R e n d e r A c t i v i t y "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   }   }   }   }   }   e r    t r u e   e    t r u e   s    t r u e   " ê   t r u e   a Ä     :   2   }   }   }   }   }   g g e X   i o n s "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   }   }   }   4 8 ˇˇˇlh ∞Ú ê«ÖCˇˇˇlh (í  Än…jÄ   " E 2 2 4 s V g J q H g Z z Y O p L b P K u j 4 S I A w y t Q f l J V J p I N d O 0 X Y = "             ConfigIds   :   ‡ˇˇˇQ W A V E   d r i v e r     à˚ˇˇ∞∞    @± »h àj òi (π (í ÿw »í ñ ® Ä2  ≤ (' j ‡j X{ ï  ∫ êk ¯| ¿‡ ®~ ‡ 8k »≤ Ä¥ µ ∏ó à∂ X‡ »∫ 0Ä Ç XÉ Ëv –} Ñ `Ñ pl 0Ö êà ®† ÄÄ  0 p0 ° ∏0 xn 1 `o X1 ®1 ∏o p ¯· t ‡® Pã òã Pr på Ëå ‡t Ë3 ê4 `© †´  Ø –u @ú ‡© ê´ 8Æ ÿ≤ ∑  ¿ p¿ † ≈ Ë  0 0 ‡ †  P ¯ 	 @
  ‡ 0  X ¯ ¿ ¯5 9 Ë9 ¿: ê; `< @=   ò> x? X@ (A  B @ `C 0D E ËE ¿F êG `H HI hv òI @z  { | Ë| »} Ë~ » ®Ä àÅ XÇ 8É Ñ HÖ (Ü ÄÜ (à Äà pº PΩ ∞Ω (ﬁ ﬂ ¿Â ®Ê ÄÁ pË »Ë òÍ ÿÎ ∏Ï ‡Ì ¿Ó ®Ô ê ÄÒ hˆ H˜ (¯ ˘ `Ú @Û (Ù ı ı 0ˇ ¥ ¯¥ ‡µ –∂ ∏∑ ®∏ Ë˘ êˇ Ä 0ª ¿ êº hΩ @æ Ä√ ê  `À 0Ã ø ‡ø ¿¿ ®¡ ê¬  Õ ËÕ 0œ òœ – ê≤ ≤ @á 0¢ @w m R e g i s t r y : : D a t a | u i n t 6 4 _ t | - 5 8 6 1 6 8 0 9 6 8 3 0 2 4 5 0 8 0 5 ; b o o l | 1   se–   vk* Ä   8ˆ     C Microsoft.Office.AppCompat.AppraiserLogger| u i à   M s hbin 	  0                      X˘ˇˇ{   " S u b N a m e s p a c e s "   :   {   " C o a u t h "   :   {   " E v e n t s "   :   {   " A d j u s t "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " C l o s e W o r k b o o k "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " C o a u t h E n a b l e d "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " D e s k t o p T r a n s i t i o n L o c k O n E d i t W o r k b o o k "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " D o w n l o a d "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " G e n e r a t e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " M e r g e O p "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " N o t C o a u t h o r a b l e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " R e l o a d O r F o r k "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " O p e n F i l e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " M e r g e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " E d i t o r P r e s e n c e R e c e i v e d "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " M e r g e C o n f l i c t R e s o l u t i o n E v e n t "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " A p p l y "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   } ,   " C o m m a n d "   :   {   " E v e n t s "   :   {   " R e f r e s h A l l "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " R e f E x t D a t a "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   } ,   " F i l e S a v e "   :   {   " E v e n t s "   :   {   " S a v e A s S a v e F i l e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   }   }   }     :      ∏g  `˚ˇˇ1 7 7 9   5 0 , 1 7 1 5   5 0 , 1 7 2 3   5 0 , 1 7 8 0   5 0 , 1 7 1 6   5 0 , 1 7 1 7   5 0 , 1 7 1 8   5 0 , 1 7 1 9   5 0 , 1 7 2 0   5 0 , 1 7 2 1   5 0 , 1 7 2 2   5 0 , 1 7 2 4   5 0 , 1 7 2 5   5 0 , 1 7 2 6   5 0 , 1 7 2 7   5 0 , 1 7 2 8   5 0 , 1 7 2 9   5 0 , 1 7 3 0   5 0 , 1 7 3 1   5 0 , 1 7 3 2   5 0 , 1 7 3 3   5 0 , 1 7 3 4   5 0 , 1 7 3 5   5 0 , 1 7 3 6   5 0 , 1 3 2 9   1 0 , 1 7 3 7   5 0 , 1 7 3 8   5 0 , 1 7 3 9   5 0 , 1 7 4 0   5 0 , 1 7 4 1   5 0 , 1 7 4 2   5 0 , 1 7 4 3   5 0 , 1 7 4 4   5 0 , 1 7 4 5   5 0 , 1 7 4 6   5 0 , 1 7 4 8   5 0 , 1 7 4 9   5 0 , 1 7 5 0   5 0 , 1 7 5 1   5 0 , 1 7 5 2   5 0 , 1 7 5 3   5 0 , 1 7 5 5   5 0 , 1 7 5 6   5 0 , 1 7 5 7   5 0 , 1 7 5 8   5 0 , 1 7 5 9   5 0 , 1 7 6 0   5 0 , 1 7 6 1   5 0 , 1 7 6 2   5 0 , 1 7 6 3   5 0 , 1 7 6 4   5 0 , 1 7 6 5   5 0 , 1 7 6 6   5 0 , 1 7 6 7   5 0 , 1 7 6 8   5 0 , 1 7 6 9   5 0 , 1 7 7 0   5 0 , 1 7 7 1   5 0 , 1 7 7 2   5 0 , 1 7 7 3   5 0 , 1 7 7 4   5 0 , 1 7 7 5   5 0 , 1 7 7 6   5 0 , 1 7 7 7   5 0 , 1 7 7 8   5 0 , 1 7 8 1   5 0 , 1 3 2 9   1 0 0 , 9 4 1   1 0 , 1 3 2 9   1 5 , 9 4 1   1 5 , 1 3 2 9   5 0 , 9 4 1   6 , 1 3 2 9   6 , 1 6 0 3   5 0   t r ÿ  t o r "   :   {   " E v e n t F l a g "   :   2   } ,   " A d d C l o u d F o n t R e s o u r c e "   :   {   " E v e n t F l a g "   :   2   }   }   } ,   " R e s o u r c e C l i e n t "   :   {   " E v e n t s "   :   {   " R e a d F o n t E l e m e n t s "   :   {   " E v e n t F l a g "   :   2 5 6   } ,   " P u r g e M u l t i p l e "   :   {   " E v e n t F l a g "   :   2   } ,   " R e a d R e s o u r c e M e t a D a t a "   :   {   " E v e n t F l a g "   :   2   } ,   " W r i t e R e s o u r c e M e t a D a t a "   :   {   " E v e n t F l a g "   :   2   } ,   " D e s e r i a l i z e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " R e q u e s t R e s o u r c e I n t e r n a l "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " P r o c e s s C a t a l o g R e s p o n s e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   } ,   " P r o c e s s R e s o u r c e R e s p o n s e "   :   {   " E v e n t F l a g "   :   4 8 8 9 6   }   }   } ,   " R i c h E d i t "   :   {   " E v e INDX( 	 m=2           (   8   Ë       M                                       i õﬁ\\g‘∆¨a\g‘∆¨a\g‘õﬁ\\g‘                     { C B 3 3 D ~ 1 . T M               - 8 d 6 1 - e c d 5 0 5 f e f 3 d f } ~ Ì    h R         i õﬁ\\g‘∆¨a\g‘∆¨a\g‘õﬁ\\g‘      