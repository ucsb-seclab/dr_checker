import React from 'react';
import PropTypes from 'prop-types';
import Dialog, { DialogTitle } from 'material-ui/Dialog';
import Typography from 'material-ui/Typography';


function SummaryInfo(props) {
  const { ...other } = props;
  return (
    
    <Dialog onRequestClose={props.onRequestClose} {...other}>
      <DialogTitle>{props.title}</DialogTitle>
      <div>
        <Typography paragraph>
          {props.content}
        </Typography>
      </div>
    </Dialog>
  );
}

SummaryInfo.propTypes = {
  title: PropTypes.string.isRequired,
  content: PropTypes.string.isRequired,
  onRequestClose: PropTypes.func.isRequired,
};

export default SummaryInfo;
