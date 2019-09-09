import { Component, OnInit } from '@angular/core';
import { HttpClient } from "@angular/common/http";
import { environment } from "../../environments/environment";

@Component({
  selector: 'app-config',
  templateUrl: './config.component.html'
})
export class ConfigComponent implements OnInit {

  public config: Config;
  public saving = false;

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.http.get(environment.backend + '/config').subscribe((data: Config) => {
      this.config = data;
    })
  }

  save() {
    this.saving = true;
    this.http.post(environment.backend + '/config', this.config).subscribe((data: Config) => {
      this.saving = false;
    })
  }

}

export interface Config {
  deviceName: string;
  wifiSsid: string;
  wifiPass: string;
  mqttHost: string;
  mqttPort: number;
  mqttUser: string;
  mqttPass: string;
}
