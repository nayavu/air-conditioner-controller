import { Component, OnInit } from '@angular/core';
import { HttpClient } from "@angular/common/http";
import { environment } from "../../environments/environment";
import { Config } from "../config/config.component";

@Component({
  selector: 'app-devices',
  templateUrl: './devices.component.html'
})
export class DevicesComponent implements OnInit {

  public aircond: AirCond;
  public status = '';

  constructor(private http: HttpClient) { }

  ngOnInit() {
    this.http.get(environment.backend + '/devices/aircond').subscribe((data: AirCond) => {
      this.aircond = data;
    })
  }

  save() {
    this.status = 'saving';
    this.http.post(environment.backend + '/devices/aircond', this.aircond).subscribe(
        (data: Config) => {
          this.status = 'saved';
        },
        error => {
          this.status = 'error';
        });
  }
}

export interface AirCond {
  power: boolean,
  t: number,
  mode: 'auto' | 'heat' | 'cool' | 'dry',
  fan: 'auto' | '1' | '2' | '3' | '4' | '5',
  swing: 'auto' | '1' | '2' | '3' | '4' | '5',
  profile: 'normal' | 'quiet' | 'boost'
}
